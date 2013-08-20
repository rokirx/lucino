#include "lucene.h"
#include "fs_field.h"
#include "lcn_store.h"
#include "index_writer.h"
#include "io_context.h"

#define BITS2BYTE( x )  (((x)>>3) + ((x)%8 ? 1 : 0))

static apr_status_t
lcn_directory_fs_field_init( lcn_directory_fs_field_t *field,
                             const char *name,
                             unsigned int docs_count,
                             unsigned int field_size,
                             apr_pool_t *pool );

static apr_status_t
lcn_directory_fs_field_set_int_value( lcn_fs_field_t *field,
                                      unsigned int val,
                                      unsigned int doc_id );

static apr_status_t
lcn_directory_fs_field_value( lcn_fs_field_t *base_field,
                              char *val,
                              unsigned int doc_id );

static apr_status_t
lcn_directory_fs_field_set_value( lcn_fs_field_t *field,
                                  const char *val,
                                  unsigned int doc_id );

static apr_status_t
lcn_directory_fs_field_commit( lcn_fs_field_t *base_field,
                               apr_pool_t *pool );

static apr_status_t
lcn_directory_fs_field_close( lcn_fs_field_t *base_field,
                              apr_pool_t *pool );

/* END of static declarations */



const char *
lcn_fs_field_name( lcn_fs_field_t *field )
{
    return field->name;
}

unsigned int
lcn_fs_field_docs_count( lcn_fs_field_t *field )
{
    return field->docs_count;
}

unsigned int
lcn_fs_field_data_size( lcn_fs_field_t *field )
{
    return field->data_size;
}

static apr_status_t
lcn_fs_accessor_get_binary_value( lcn_field_accessor_t *accessor,
                                  char **val,
                                  unsigned int *len,
                                  unsigned int doc_id,
                                  apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_fs_field_t *field = (lcn_fs_field_t*) accessor;
        char *buf = (char*) apr_palloc( pool, *len = ((BITS2BYTE(field->data_size) * sizeof(char))));

        LCNPV( buf, APR_ENOMEM );
        LCNCE( lcn_fs_field_value( field, buf, doc_id ));

        *val = buf;
    }
    while(0);

    return APR_SUCCESS;
}

static apr_status_t
lcn_fs_accessor_get_int_value( lcn_field_accessor_t *accessor,
                               unsigned int *val,
                               unsigned int doc_id )
{
    apr_status_t s;
    lcn_fs_field_t *field = (lcn_fs_field_t*) accessor;
    LCNCR( lcn_fs_field_int_value( field, val, doc_id ) );
    return s;
}

static apr_status_t
lcn_fs_field_open_input( lcn_directory_fs_field_t *field,
                         lcn_bool_t create_file )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    do
    {
        char *file_name;
        lcn_bool_t file_exists;

        LCNCE( apr_pool_create( &cp, field->parent.pool ));
        LCNPV( field->directory, LCN_ERR_INVALID_ARGUMENT );
        LCNPV( file_name = apr_pstrcat( field->parent.pool, field->parent.name, ".fsf", NULL ), APR_ENOMEM );
        LCNCE( lcn_directory_file_exists( field->directory, file_name, &file_exists ));

        if ( create_file && (! file_exists) )
        {
            lcn_index_output_t *os;

            LCNCE( lcn_directory_create_output( field->directory,
                                                &os,
                                                file_name,
                                                cp ));

            LCNCE( lcn_fs_field_write_info( field, os ));
            LCNCE( lcn_index_output_close( os ));
        }

        LCNCE( lcn_directory_open_input( field->directory,
                                         &(field->istream),
                                         file_name,
                                         LCN_IO_CONTEXT_READONCE,
                                         field->parent.pool ));
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}


static apr_status_t
lcn_fs_field_init_field_info_from_stream( lcn_directory_fs_field_t *field,
                                          lcn_index_input_t *is,
                                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        int version;
        char *field_name;
        unsigned int dlen, fname_len;
        int docs_count;
        int data_size;
        char *default_value;

        LCNCE( lcn_index_input_read_int( is, &version ));
        LCNASSERT( 1 == version, LCN_ERR_INVALID_FILE_VERSION_NUMBER);

        LCNCE( lcn_index_input_read_string( is, &field_name, &fname_len, pool ));
        LCNCE( lcn_index_input_read_int( is, &docs_count ));
        LCNCE( lcn_index_input_read_int( is, &data_size));

        /* read default value */

        dlen = BITS2BYTE( data_size );
        LCNPV( default_value = apr_pcalloc( pool, dlen ), APR_ENOMEM );
        LCNCE( lcn_index_input_read_bytes( is, default_value, 0, &dlen ));

        /* this function is used to skip first entries of the */
        /* fs field file, then it is ok for field to be NULL  */

        if ( NULL != field )
        {
            LCNCE( lcn_directory_fs_field_init( field, field_name, docs_count, data_size, pool ));
            LCNCE( lcn_directory_fs_field_set_default_value( field, default_value ));
        }
    }
    while(0);

    return s;
}

unsigned int
lcn_fs_field_default_to_int( unsigned int data_size,
                             char *default_value )
{
    unsigned int result = 0;

    if ( 1 == data_size )
    {
        return (unsigned int) ( default_value ? default_value[0] & 1 : 0 );
    }
    else
    {
        if ( NULL != default_value )
        {
            unsigned int i;
            unsigned int size = BITS2BYTE( data_size );

            for( i = 0; i < size; i++)
            {
                result |= (((unsigned int)((unsigned char) default_value[ i ])) << (8*i));
            }
        }
    }

    return result;
}


static apr_status_t
lcn_directory_fs_field_int_value( lcn_fs_field_t *base_field,
                                  unsigned int *val,
                                  unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;
    lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) base_field;

    do
    {
        /* return default value */

        if ( doc_id >= field->parent.docs_count ||
             ( NULL == field->buf && NULL == field->istream && NULL == field->directory ))
        {
            *val = lcn_fs_field_default_to_int( field->parent.data_size, field->parent.default_value );
            break;
        }

        if ( NULL == field->buf )
        {
            if ( NULL == field->istream )
            {
                LCNCE( lcn_fs_field_open_input( field, LCN_FALSE /* create file */ ));
                LCNCE( lcn_fs_field_init_field_info_from_stream( NULL, field->istream, field->parent.pool ));
                field->base_offset = lcn_index_input_file_pointer( field->istream );
            }

            /* position the stream and read value */

            if ( 1 == field->parent.data_size )
            {
                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + (doc_id>>3) ));
                LCNCE( lcn_index_input_read_byte( field->istream, ((unsigned char*)field->parent.tmp_value) ));

                *val = (0 != (field->parent.tmp_value[0] & (1<<(doc_id & 7))));
            }
            else if ( 0 == ( field->parent.data_size % 8 ))
            {
                apr_off_t offset = doc_id * (field->parent.data_size>>3);
                unsigned int len = field->parent.data_size >> 3;
                unsigned int i;

                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + offset ));
                LCNCE( lcn_index_input_read_bytes( field->istream,
                                               field->parent.tmp_value,
                                               0,
                                               &len ));

                *val = 0;

                for( i = 0; i < len; i++ )
                {
                    *val |= ((unsigned int)((unsigned char)field->parent.tmp_value[i])) << (i<<3);
                }
            }
            else
            {
                apr_off_t offset = (doc_id * field->parent.data_size ) >> 3;
                unsigned int len = BITS2BYTE( field->parent.data_size );
                unsigned int i;
                unsigned int start_bit;
                apr_status_t stat;

                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + offset ));
                LCNCE( lcn_index_input_read_bytes( field->istream,
                                               field->parent.tmp_value,
                                               0,
                                               &len ));

                /* we need one more byte to be sure we have read enough of them */
                /* except we've already read them all                           */

                stat = lcn_index_input_read_byte( field->istream, ((unsigned char*)field->parent.tmp_value) + len );

                if ( !(LCN_ERR_READ_PAST_EOF == stat || APR_SUCCESS == stat) )
                {
                    s = stat;
                    break;
                }

                start_bit = (doc_id * field->parent.data_size) & 7;

                *val = 0;

                for( i = 0; i < field->parent.data_size; i++ )
                {
                    unsigned int boffset = start_bit + i;
                    *val |= ((unsigned int)((field->parent.tmp_value[boffset>>3] & ( 1 << (boffset&7) )) ? 1 : 0)) << i;
                }
            }
        }
        else /* field->buf != NULL */
        {
            if ( 1 == field->parent.data_size )
            {
                *val = (((field->buf[ ((unsigned int)doc_id)  >> 3 ] & (1 << ((unsigned int)doc_id) ))) != 0);
            }
            else if ( 0 == ( field->parent.data_size % 8 ))
            {
                unsigned int size = field->parent.data_size >> 3;
                unsigned int i;
                *val = 0;

                for( i = 0; i < size; i++ )
                {
                    *val |= (((unsigned int)(unsigned char) field->buf[ doc_id * size + i ]) << (8*i));
                }
            }
            else
            {
                unsigned int i;

                *val = 0;

                for( i = 0; i < field->parent.data_size; i++ )
                {
                    unsigned int boffset = doc_id * field->parent.data_size + i ;
                    *val |= ((unsigned int)((field->buf[boffset>>3] & ( 1 << (boffset&7) )) ? 1 : 0)) << i;
                }
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_directory_fs_field_init( lcn_directory_fs_field_t *field,
                             const char *name,
                             unsigned int docs_count,
                             unsigned int field_size,
                             apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        field->parent.name       = apr_pstrdup( pool, name );
        field->parent.pool       = pool;
        field->parent.docs_count = docs_count;
        field->parent.data_size  = field_size;

        LCNPV( field->parent.tmp_value = apr_pcalloc( pool, 1 + BITS2BYTE(field_size)), APR_ENOMEM );
        field->buf_size = 0;

        field->parent.accessor.get_binary_value = lcn_fs_accessor_get_binary_value;
        field->parent.accessor.get_int_value    = lcn_fs_accessor_get_int_value;


        field->parent.int_value     = lcn_directory_fs_field_int_value;
        field->parent.set_int_value = lcn_directory_fs_field_set_int_value;
        field->parent.field_value   = lcn_directory_fs_field_value;
        field->parent.set_value     = lcn_directory_fs_field_set_value;
        field->parent.commit        = lcn_directory_fs_field_commit;
        field->parent.close         = lcn_directory_fs_field_close;
        field->parent.type          = LCN_DIRECTORY_FS_FIELD;
    }
    while(0);

    return s;
}

/**
 * @brief   Fills the internal field buffer with the contents of
 *          the stream
 *
 * @param field    Field to fill
 * @param istream  Stream to read the data
 */
static apr_status_t
lcn_fs_field_fill_buffer( lcn_directory_fs_field_t *field,
                          lcn_index_input_t *istream )
{
    apr_status_t s;

    do
    {
        unsigned int len = BITS2BYTE( field->parent.docs_count * field->parent.data_size );

        LCNPV( field->buf = (char*) apr_palloc( field->parent.pool, len ), APR_ENOMEM );

        field->buf_size = len;

        LCNCE( lcn_index_input_read_bytes( istream,
                                       field->buf,
                                       0, /* offset */
                                       &len ));
    }
    while(0);

    return s;
}

lcn_bool_t
lcn_fs_field_is_modified( lcn_fs_field_t* fs_field )
{
    if ( fs_field->type == LCN_MULTI_FS_FIELD )
    {
        lcn_multi_fs_field_update_modified_flag( (lcn_multi_fs_field_t*) fs_field );
    }

    return fs_field->is_modified;
}

apr_status_t
lcn_fs_field_to_field_info( lcn_fs_field_t* fs_field,
                            lcn_field_info_t **field_info,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_field_info_t *fi;

        LCNPV( fi = (lcn_field_info_t*) apr_pcalloc( pool, sizeof(lcn_field_info_t) ), APR_ENOMEM );

        fi->name = apr_pstrdup( pool, fs_field->name );
        fi->bits = LCN_FIELD_INFO_FIXED_SIZE;

        *field_info = fi;
    }
    while(0);

    return s;
}


apr_status_t
lcn_fs_field_to_field( lcn_fs_field_t* fs_field,
                       lcn_field_t **field,
                       unsigned int doc_id,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_field_create_fixed_size( field,
                                            fs_field->name,
                                            NULL,
                                            fs_field->default_value,
                                            fs_field->data_size,
                                            pool ));
        (*field)->is_lazy = LCN_TRUE;
        (*field)->accessor = (lcn_field_accessor_t*) fs_field;
        (*field)->doc_id = doc_id;
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_update_fields_def( lcn_directory_fs_field_t *field,
                                lcn_directory_t* directory,
                                apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_index_output_t *info_os = NULL;

    do
    {
        apr_hash_t *hash = apr_hash_make( pool );
        apr_hash_index_t *hi;
	lcn_list_t *sorted_names;
	int i;

        LCNPV( hash, APR_ENOMEM );
        LCNCE( lcn_directory_fs_field_read_field_infos( hash, directory, pool ));

        apr_hash_set( hash, field->parent.name, strlen(field->parent.name), field );

        LCNCE( lcn_directory_create_output( directory,
                                            &info_os,
                                            LCN_INDEX_WRITER_FIXED_SIZE_FIELD_DEF,
                                            pool ));

        LCNCE( lcn_index_output_write_int( info_os, apr_hash_count( hash )));
	LCNCE( lcn_list_create( &sorted_names, 10, pool ));

        for( hi = apr_hash_first( pool, hash ); hi; hi = apr_hash_next( hi ))
        {
            const void *vkey;

            apr_hash_this( hi, &vkey, NULL, NULL );

	    LCNCE( lcn_list_add( sorted_names, (void*) vkey ));
        }

	LCNCE( s );

	LCNCE( lcn_list_sort_cstrings( sorted_names ));

	for( i = 0; i < lcn_list_size( sorted_names ); i++ )
	{
	    char *name = (char*) lcn_list_get( sorted_names, i );
            lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) apr_hash_get( hash, name, strlen(name) );
	    LCNCE( lcn_fs_field_write_info( field, info_os ));
	}
    }
    while(0);

    if ( NULL != info_os )
    {
        apr_status_t save_s = lcn_index_output_close( info_os );
        s = s ? s : save_s;
    }

    return s;
}


apr_status_t
lcn_directory_fs_field_read_field_infos( apr_hash_t *hash,
                                         lcn_directory_t *directory,
                                         apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_index_input_t *is = NULL;

    do
    {
        int i, field_count;
        lcn_bool_t exists;

        LCNCE( lcn_directory_file_exists ( directory, LCN_INDEX_WRITER_FIXED_SIZE_FIELD_DEF, &exists ));

        if ( ! exists )
        {
            break;
        }

        LCNCE( lcn_directory_open_input( directory,
                                         &is,
                                         LCN_INDEX_WRITER_FIXED_SIZE_FIELD_DEF,
                                         LCN_IO_CONTEXT_READONCE,
                                         pool ));

        LCNCE( lcn_index_input_read_int( is, &field_count ));

        for( i = 0; i < field_count; i++ )
        {
            lcn_directory_fs_field_t *field;

            LCNPV( field = (lcn_directory_fs_field_t*) apr_pcalloc( pool, sizeof(lcn_directory_fs_field_t)), APR_ENOMEM );
            LCNCE( lcn_fs_field_init_field_info_from_stream( field, is, pool ));
            field->directory = directory;
            apr_hash_set( hash, apr_pstrdup( apr_hash_pool_get( hash ), field->parent.name ) , strlen(field->parent.name), field );
        }
    }
    while(0);

    if ( NULL != is )
    {
        apr_status_t stat = lcn_index_input_close( is );
        s = s ? s : stat;
    }

    return s;
}

apr_status_t
lcn_directory_fs_field_create( lcn_fs_field_t **field,
                               const char *name,
                               unsigned int docs_count,
                               unsigned int field_size,
                               lcn_directory_t *directory,
                               apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_directory_fs_field_t *new_field = lcn_object_create( lcn_directory_fs_field_t, pool );

    do
    {
        LCNCE( lcn_directory_fs_field_init( new_field, name, docs_count, field_size, pool ));
        *field = (lcn_fs_field_t*) new_field;

        if ( NULL != directory )
        {
            new_field->directory = directory;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_int_to_char( unsigned int val,
                          char *buf,
                          unsigned int bit_size )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        unsigned int size = BITS2BYTE( bit_size );

        LCNASSERT( size <= sizeof(int), LCN_ERR_INVALID_ARGUMENT );

        for( i = 0; i < size; i++ )
        {
            buf[i] = (char) ((unsigned char) ((unsigned int) val >> (8*i)));
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_directory_fs_field_set_default_value( lcn_directory_fs_field_t *field,
                                          const char *default_value )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( NULL != default_value )
        {
            int bytes = BITS2BYTE(field->parent.data_size);

            LCNPR( field->parent.default_value = apr_pcalloc( field->parent.pool, bytes), APR_ENOMEM );
            memcpy( field->parent.default_value, default_value, bytes );

            if ( field->parent.data_size % 8 )
            {
                int shift = 8 - (field->parent.data_size % 8);
                field->parent.default_value[ bytes - 1 ] = ( field->parent.default_value[ bytes - 1 ] << shift) >> shift;
            }
        }
        else
        {
            field->parent.default_value = NULL;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_directory_fs_field_set_default_int_value( lcn_directory_fs_field_t *field,
                                              unsigned int default_value )
{
    apr_status_t s;

    do
    {
        char iarr[ sizeof(int) ];

        LCNCE( lcn_fs_field_int_to_char( default_value, iarr, field->parent.data_size ));
        LCNCE( lcn_directory_fs_field_set_default_value( field, default_value == 0 ? NULL : iarr ));
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_value( lcn_fs_field_t *field,
                    char *val,
                    unsigned int doc_id )
{
    return field->field_value( field, val, doc_id );
}


static apr_status_t
lcn_directory_fs_field_value( lcn_fs_field_t *base_field,
                              char *val,
                              unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;
    lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t *) base_field;

    do
    {
        /* return default value */

        if ( doc_id >= field->parent.docs_count )
        {
            if ( 1 == field->parent.data_size )
            {
                *val = ( field->parent.default_value ? field->parent.default_value[0] & 1 : 0 );
                break;
            }
            else
            {
                *val = 0;

                if ( NULL != field->parent.default_value )
                {
                    unsigned int size = BITS2BYTE( field->parent.data_size );
                    memcpy( val, field->parent.default_value, size );
                }
                break;
            }
        }

        if ( NULL == field->buf )
        {
            if ( NULL == field->istream )
            {
                LCNCE( lcn_fs_field_open_input( (lcn_directory_fs_field_t*)field, LCN_FALSE ));
                LCNCE( lcn_fs_field_init_field_info_from_stream( NULL, field->istream, field->parent.pool ));
                field->base_offset = lcn_index_input_file_pointer( field->istream );
            }

            /* position the stream and read value */

            if ( 1 == field->parent.data_size )
            {
                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + (doc_id>>3) ));
                LCNCE( lcn_index_input_read_byte( field->istream, ((unsigned char*)field->parent.tmp_value) ));

                val[0] = (0 != (field->parent.tmp_value[0] & (1<<(doc_id & 7))));
            }
            else if ( 0 == ( field->parent.data_size % 8 ))
            {
                apr_off_t offset = doc_id * (field->parent.data_size>>3);
                unsigned int len = field->parent.data_size >> 3;

                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + offset ));
                LCNCE( lcn_index_input_read_bytes( field->istream,
                                               val,
                                               0,
                                               &len ));
            }
            else
            {
                apr_off_t offset = (doc_id * field->parent.data_size) >>3;
                unsigned int len = BITS2BYTE( field->parent.data_size );
                unsigned int i;
                unsigned int start_bit;
                apr_status_t stat;

                LCNCE( lcn_index_input_seek( field->istream, field->base_offset + offset ));
                LCNCE( lcn_index_input_read_bytes( field->istream,
                                               field->parent.tmp_value,
                                               0,
                                               &len ));

                /* we need one more byte to be sure we have read enough of them */
                /* except we've already read them all                           */

                stat = lcn_index_input_read_byte( field->istream, ((unsigned char*)field->parent.tmp_value) + len );

                if ( !(LCN_ERR_READ_PAST_EOF == stat || APR_SUCCESS == stat) )
                {
                    s = stat;
                    break;
                }

                start_bit = (doc_id * field->parent.data_size) & 7;

                for( i = 0; i < field->parent.data_size; i++ )
                {
                    unsigned int boffset = start_bit + i;

                    if ( 0 == (i&7))
                    {
                        val[i>>3] = 0;
                    }

                    val[i>>3] |= ((unsigned int)((field->parent.tmp_value[boffset>>3] & ( 1 << (boffset&7))) ? 1 : 0 )) << (i&7);
                }
            }
        }
        else
        {
            if ( 1 == field->parent.data_size )
            {
                *val = (((field->buf[ ((unsigned int)doc_id)  >> 3 ] & (1 << ((unsigned int)doc_id) ))) != 0);
            }
            else if ( 0 == ( field->parent.data_size % 8 ))
            {
                unsigned int size = field->parent.data_size >> 3;
                memcpy( val, field->buf + (doc_id * size), size );
            }
            else
            {
                unsigned int i;

                for( i = 0; i < field->parent.data_size; i++ )
                {
                    unsigned int boffset = doc_id * field->parent.data_size + i ;

                    if(0 == (i&7))
                    {
                        val[i>>3] = 0;
                    }

                    val[i>>3] |= ((unsigned int)((field->buf[boffset>>3] & ( 1 << (boffset&7) )) ? 1 : 0)) << (i&7);
                }
            }
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_fs_field_int_value( lcn_fs_field_t *field,
                        unsigned int *val,
                        unsigned int doc_id )
{
    return field->int_value( field, val, doc_id );
}

static apr_status_t
lcn_directory_fs_field_set_value_impl( lcn_directory_fs_field_t *field, unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i;

        /* we may need to read the data from file first */

        if ( 0 == field->buf_size )
        {
            if ( NULL == field->istream && NULL != field->directory )
            {
                LCNCE( lcn_fs_field_open_input( field, LCN_TRUE /* create file */ ));
            }

            if ( NULL != field->istream )
            {
                if ( 0 == lcn_index_input_file_pointer( field->istream ))
                {
                    LCNCE( lcn_fs_field_init_field_info_from_stream( NULL, field->istream, field->parent.pool ));
                    LCNCE( lcn_fs_field_fill_buffer( field, field->istream ));
                }
                else
                {
                    /* assume here field is initialized! */
                    LCNCE( lcn_index_input_seek( field->istream, field->base_offset ));
                    LCNCE( lcn_fs_field_fill_buffer( field, field->istream ));
                }
            }
        }

        if ( field->buf_size < (((doc_id+1) * field->parent.data_size) >> 3) + 1 )
        {
            char *new_buf;
            unsigned int new_buf_size = 2 * ((((doc_id+1) * field->parent.data_size) >> 3) + 1);
            new_buf = apr_pcalloc( field->parent.pool, new_buf_size );
            LCNPV( new_buf, APR_ENOMEM );

            if ( NULL != field->buf )
            {
                memcpy( new_buf, field->buf, field->buf_size );
            }

            field->buf = new_buf;
            field->buf_size = new_buf_size;
        }

        if ( (NULL != field->parent.default_value) && ( field->parent.docs_count < doc_id) )
        {
            char *save_tmp = field->parent.tmp_value;

            field->parent.tmp_value = field->parent.default_value;

            for( i = field->parent.docs_count; i < doc_id; i++ )
            {
                lcn_directory_fs_field_set_value_impl( field, i );
            }

            field->parent.tmp_value = save_tmp;
        }

        for( i = 0; i < field->parent.data_size; i++ )
        {
            unsigned int bits = doc_id * field->parent.data_size + i;
            unsigned int k = bits >> 3;
            unsigned int b = bits & 7;

            field->buf[k] &= ~(1<<(((unsigned char)b)));

            if ( field->parent.tmp_value[i>>3] & ( 1 << ((unsigned int)(i&7))))
            {
                field->buf[k] |=   1<<((unsigned int)b);
            }
        }

        if ( field->parent.docs_count <= doc_id )
        {
            field->parent.docs_count = doc_id + 1;
        }

        field->parent.is_modified = LCN_TRUE;
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_set_int_value( lcn_fs_field_t *field,
                            unsigned int val,
                            unsigned int doc_id )
{
    return field->set_int_value( field, val, doc_id );
}

apr_status_t
lcn_fs_field_set_default_int_value( lcn_fs_field_t *field,
                                    unsigned int default_value )
{
    if ( LCN_DIRECTORY_FS_FIELD == field->type )
    {
        return lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field,
                                                             default_value );
    }

    return LCN_ERR_UNSUPPORTED_OPERATION;
}


static apr_status_t
lcn_directory_fs_field_set_int_value( lcn_fs_field_t *field,
                                      unsigned int val,
                                      unsigned int doc_id )
{
    apr_status_t s;
    lcn_directory_fs_field_t *f = (lcn_directory_fs_field_t*) field;

    LCNCR( lcn_fs_field_int_to_char( val, f->parent.tmp_value, f->parent.data_size ));
    return lcn_directory_fs_field_set_value_impl( f, doc_id );
}

apr_status_t
lcn_fs_field_set_value( lcn_fs_field_t *field,
                        const char *val,
                        unsigned int doc_id )
{
    return field->set_value( field, val, doc_id );
}

static apr_status_t
lcn_directory_fs_field_set_value( lcn_fs_field_t *field,
                                  const char *val,
                                  unsigned int doc_id )
{
    lcn_directory_fs_field_t* f = (lcn_directory_fs_field_t*) field;
    memcpy( f->parent.tmp_value, val, BITS2BYTE(f->parent.data_size));
    return lcn_directory_fs_field_set_value_impl( f, doc_id );
}

const char*
lcn_fs_field_default_val( const lcn_fs_field_t *field )
{
    return field->default_value;
}

apr_status_t
lcn_fs_field_write_info( lcn_directory_fs_field_t *field,
                         lcn_index_output_t *ostream )
{
    apr_status_t s;

    LCNCR( lcn_index_output_write_int( ostream, 1 )); /* version number */
    LCNCR( lcn_index_output_write_string( ostream, field->parent.name ));
    LCNCR( lcn_index_output_write_int( ostream, field->parent.docs_count ));
    LCNCR( lcn_index_output_write_int( ostream, field->parent.data_size ));
    LCNCR( lcn_index_output_write_bytes( ostream, field->parent.default_value, BITS2BYTE( field->parent.data_size )));

    return s;
}

apr_status_t
lcn_fs_field_write_content( lcn_directory_fs_field_t *field,
                            lcn_index_output_t *ostream )
{
    return lcn_index_output_write_bytes( ostream,
                                    NULL == field->buf ?
                                    field->parent.default_value : field->buf,
                                    BITS2BYTE( field->parent.docs_count * field->parent.data_size ));
}

apr_status_t
lcn_fs_field_close_fields_in_hash( apr_hash_t *fields )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    if ( NULL == fields || 0 == apr_hash_count( fields ))
    {
        return s;
    }

    do
    {
        apr_hash_index_t *hi;

        LCNCE( apr_pool_create( &cp, apr_hash_pool_get( fields )));

        for( hi = apr_hash_first( cp, fields); hi; hi = apr_hash_next( hi ))
        {
            lcn_fs_field_t *fs_field;
            void *vval;

            apr_hash_this( hi, NULL, NULL, &vval );
            fs_field = (lcn_fs_field_t *) vval;
            LCNCE( lcn_fs_field_close( fs_field, cp ));
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_close( lcn_fs_field_t *field,
                    apr_pool_t *pool )
{
    return field->close( field, pool );
}

static apr_status_t
lcn_directory_fs_field_close( lcn_fs_field_t *base_field,
                              apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) base_field;

    do
    {
        LCNCE( lcn_fs_field_commit( base_field, pool ));

        if ( NULL != field->istream )
        {
            LCNCE( lcn_index_input_close( field->istream ));
        }

        if ( NULL != field->directory )
        {
            LCNCE( lcn_directory_close( field->directory ));
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_fs_field_commit( lcn_fs_field_t *field,
                     apr_pool_t *pool )
{
    return field->commit( field, pool );
}


static apr_status_t
lcn_directory_fs_field_commit( lcn_fs_field_t *base_field,
                               apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) base_field;

    if ( ! lcn_fs_field_is_modified( base_field ))
    {
        return s;
    }

    do
    {
        char *file_name;
        lcn_index_output_t *os;

        LCNPV( file_name = apr_pstrcat( pool, field->parent.name, ".fsf", NULL ), APR_ENOMEM );
        LCNPV( field->directory, LCN_ERR_INVALID_ARGUMENT );

        LCNCE( lcn_directory_create_output( field->directory,
                                            &os,
                                            file_name,
                                            pool ));

        LCNCE( lcn_fs_field_write_info( field, os ));
        LCNCE( lcn_fs_field_write_content( field, os ));
        LCNCE( lcn_index_output_close( os ));
        LCNCE( lcn_fs_field_update_fields_def( field, field->directory, pool ));

        field->parent.is_modified = LCN_FALSE;
    }
    while(0);

    return s;
}

apr_status_t
lcn_directory_fs_field_init_buffer( lcn_directory_fs_field_t *field )
{
    apr_status_t s;

    if ( NULL != field->buf )
    {
        return APR_SUCCESS;
    }

    do
    {
        if ( NULL == field->istream )
        {
            LCNCE( lcn_fs_field_open_input( field, LCN_FALSE /* create file */ ));
            LCNCE( lcn_fs_field_init_field_info_from_stream( NULL, field->istream, field->parent.pool ));
            field->base_offset = lcn_index_input_file_pointer( field->istream );
        }

        LCNCE( lcn_fs_field_fill_buffer( field, field->istream ));
    }
    while(0);

    return s;
}

apr_status_t
lcn_directory_fs_field_read( lcn_directory_fs_field_t **field,
                             const char *name,
                             lcn_index_input_t *istream,
                             apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_directory_fs_field_t *new_field;

        LCNPV( new_field = (lcn_directory_fs_field_t*) apr_pcalloc( pool, sizeof(lcn_directory_fs_field_t)), APR_ENOMEM );

        new_field->parent.pool = pool;

        LCNCE( lcn_fs_field_init_field_info_from_stream( new_field, istream, pool ));
        LCNCE( lcn_fs_field_fill_buffer( new_field, istream ));

        *field = new_field;
    }
    while(0);

    return s;
}

const char*
lcn_fs_field_next_field_name( lcn_list_t *hash_list,
                              apr_pool_t *pool )
{
    unsigned int i;

    for( i = 0; i < lcn_list_size( hash_list ); i++ )
    {
        apr_hash_t *hash = (apr_hash_t*) lcn_list_get( hash_list, i );
        apr_hash_index_t *hi;

        for( hi = apr_hash_first( pool, hash); hi; hi = apr_hash_next( hi ))
        {
            const void *vkey;
            apr_hash_this( hi, &vkey, NULL, NULL );
            return (char*) vkey;
        }
    }

    return NULL;
}

lcn_bool_t
lcn_fs_field_info_is_equal( lcn_fs_field_t *fa,
                            lcn_fs_field_t *fb )
{
    unsigned int dlen, i;

    if ( strcmp( fa->name, fb->name ) ||
         fa->data_size != fb->data_size )
    {
        return LCN_FALSE;
    }

    if ( NULL == fa->default_value &&
         NULL == fb->default_value )
    {
        return LCN_TRUE;
    }

    if ( NULL == fa->default_value ||
         NULL == fb->default_value )
    {
        return LCN_FALSE;
    }

    dlen = BITS2BYTE(fa->data_size);

    for( i = 0; i < dlen; i++ )
    {
        if ( fa->default_value[ i ]  != fb->default_value[ i ] )
        {
            return LCN_FALSE;
        }
    }

    return LCN_TRUE;
}

static lcn_bool_t
lcn_fs_field_is_consistently_defined( const char* field_name,
                                      lcn_list_t *hash_list )
{
    unsigned int i;
    lcn_fs_field_t *field = NULL;

    for( i = 0; i < lcn_list_size( hash_list ); i++ )
    {
        apr_hash_t *hash = (apr_hash_t*) lcn_list_get( hash_list, i );
        lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( hash, field_name, strlen(field_name) );

        if ( NULL == f )
        {
            continue;
        }

        if ( NULL == field )
        {
            field = f;
            continue;
        }

        if ( ! lcn_fs_field_info_is_equal( field, f ))
        {
            return LCN_FALSE;
        }
    }

    return LCN_TRUE;
}

static void
lcn_fs_field_remove( const char *field_name,
                     lcn_list_t *hash_list )
{
    unsigned int i;

    for( i = 0; i < lcn_list_size( hash_list ); i++ )
    {
        apr_hash_t *hash = (apr_hash_t*) lcn_list_get( hash_list, i );
        apr_hash_set( hash, field_name, strlen(field_name), NULL );
    }
}

/**
 * @brief Merges the fields in the directory list into the
 *        first directory of the list
 *
 * @param field_name  Name of the fs field to be merged
 * @param dir_list    List of directories with the fields to be merged
 * @param fi_list     List of field infos, one hash per directory
 * @param pool        APR-pool
 */
static apr_status_t
lcn_fs_field_do_merge( const char* field_name,
                       lcn_list_t *dir_list,
                       lcn_list_t *fi_list,
                       apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    lcn_directory_fs_field_t *main_field = NULL;
    unsigned int max_doc = 0;

    do
    {
        for( i = 0; i < lcn_list_size( fi_list ); i++ )
        {
            apr_hash_t *hash = (apr_hash_t*) lcn_list_get( fi_list, i );
            lcn_directory_fs_field_t *f = (lcn_directory_fs_field_t*) apr_hash_get( hash, field_name, strlen(field_name) );
            unsigned int cur_max_doc;
            unsigned int k;
            char *buf;

            lcn_directory_t *dir = (lcn_directory_t*) lcn_list_get( dir_list, i );
            lcn_index_reader_t *reader;

            LCNCE( lcn_index_reader_create_by_directory( &reader, dir, LCN_TRUE, pool ));
            cur_max_doc = lcn_index_reader_max_doc( reader );
            LCNCE( lcn_index_reader_close( reader ));

            if ( NULL == f )
            {
                max_doc += cur_max_doc;
                continue;
            }

            if ( NULL == main_field )
            {
                lcn_fs_field_t *new_field;

                LCNCE( lcn_directory_fs_field_create( &new_field,
                                                      field_name,
                                                      0,
                                                      lcn_fs_field_data_size( (lcn_fs_field_t*) f ),
                                                      (lcn_directory_t*) lcn_list_get( dir_list, 0 ),
                                                      pool ));

                main_field = (lcn_directory_fs_field_t *) new_field;
                LCNCE( lcn_directory_fs_field_set_default_value( main_field, ((lcn_fs_field_t*) f)->default_value ));
            }

            /* copy data */

            LCNPV( buf = apr_pcalloc( pool, 1 + BITS2BYTE(f->parent.data_size)), APR_ENOMEM );

            for( k = 0; k < cur_max_doc; k++ )
            {
                LCNCE( lcn_fs_field_value( (lcn_fs_field_t*) f, buf, k ));
                LCNCE( lcn_fs_field_set_value( (lcn_fs_field_t*) main_field, buf, max_doc + k ));
            }

            max_doc += cur_max_doc;
        }

        LCNCE( lcn_fs_field_commit( (lcn_fs_field_t*) main_field, pool ));
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_field_merge_indexes( lcn_directory_t *dir,
                            lcn_list_t *dir_list,
                            apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    LCNCR( apr_pool_create( &cp, pool ));

    do
    {
        unsigned int i;
        lcn_list_t *fi_list, *ldir;
        const char *field_name;

        /* set up ldir (list of directories): first element is the        */
        /* target directory, the other elements are the directories to be */
        /* merged                                                         */

        LCNCE( lcn_list_create( &ldir, 10, cp ));
        LCNCE( lcn_list_add( ldir, dir ));
        LCNCE( lcn_list_append( ldir, dir_list ));
        LCNCE( lcn_list_create( &fi_list, 10, cp ));

        /* read field infos for fixed sized fields from all directories */
        /* keep the information in a list of hash-per-directory         */

        for( i = 0; i < lcn_list_size( ldir ); i++ )
        {
            lcn_directory_t *dir = (lcn_directory_t*) lcn_list_get( ldir, i );
            apr_hash_t *hash = apr_hash_make( cp );
            LCNPV( hash, APR_ENOMEM );

            LCNCE( lcn_directory_fs_field_read_field_infos( hash, dir, cp ));
            LCNCE( lcn_list_add( fi_list, hash ));
        }

        LCNCE( s );

        while( (NULL != ( field_name = lcn_fs_field_next_field_name( fi_list, cp ))))
        {
            if ( ! lcn_fs_field_is_consistently_defined( field_name, fi_list ))
            {
                s = LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION;
                break;
            }

            LCNCE( lcn_fs_field_do_merge( field_name, ldir, fi_list, cp ));

            lcn_fs_field_remove( field_name, fi_list );
        }

        LCNCE( s );
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}
