#include "lucene.h"
#include "lcn_store.h"
#include "lcn_index.h"
#include "document.h"
#include "field.h"
#include "field_infos.h"

/**
 * contains implementation of FieldInfo and FieldInfos:
 *
 * field_info constructor is inlined in lcn_field_infos_add_field_info,
 * boolean members are mapped to a bitfield
 *
 * FieldInfos.addIndexed not implemented. Add fields one by one.
 * FieldInfos.add(Collection...) not implemented. Add fields one by one.
 * FieldInfos.add methods not implemented. Use lcn_field_infos_add_field_info with a bit mask.
 */


const char*
lcn_field_info_name( lcn_field_info_t *field_info )
{
    return field_info->name;
}

lcn_bool_t
lcn_field_info_is_indexed( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_IS_INDEXED );
}

lcn_bool_t
lcn_field_info_omit_norms( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_OMIT_NORMS );
}

lcn_bool_t
lcn_field_info_fixed_size( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_FIXED_SIZE );
}

lcn_bool_t
lcn_field_info_store_term_vector( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_STORE_TERM_VECTOR );
}

lcn_bool_t
lcn_field_info_store_offset_with_term_vector( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR );
}

lcn_bool_t
lcn_field_info_store_position_with_term_vector( lcn_field_info_t *field_info )
{
    return (lcn_bool_t) ( field_info->bits & LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR );
}

unsigned int
lcn_field_infos_size( lcn_field_infos_t *field_infos )
{
    return field_infos->size;
}


apr_status_t
lcn_field_infos_by_number ( lcn_field_infos_t *self,
                            lcn_field_info_t **field_info,
                            unsigned int n )
{
    lcn_field_info_t *fi = self->first_info;

    while ( fi != NULL )
    {
        if ( n == fi->number )
        {
            *field_info = fi;
            return APR_SUCCESS;
        }

        fi = fi->next;
    }

    return LCN_ERR_FIELD_NOT_FOUND;
}

apr_status_t
lcn_field_infos_nth_info ( lcn_field_infos_t *field_infos,
                           lcn_field_info_t **field_info,
                           unsigned int n )
{
    lcn_field_info_t *fi = field_infos->first_info;
    unsigned int i = 0;

    while ( fi != NULL )
    {
        if ( i++ == n )
        {
            *field_info = fi;
            return APR_SUCCESS;
        }

        fi = fi->next;
    }

    return LCN_ERR_FIELD_NOT_FOUND;
}

apr_status_t
lcn_field_infos_by_name( lcn_field_infos_t *self,
                         lcn_field_info_t **field_info,
                         const char *name )
{
    lcn_field_info_t *fi = self->first_info;

    while( fi != NULL )
    {
        if ( strcmp( fi->name, name ) == 0 )
        {
            *field_info = fi;
            return APR_SUCCESS;
        }

        fi = fi->next;
    }

    return LCN_ERR_FIELD_NOT_FOUND;
}


apr_status_t
lcn_field_infos_name_by_number( lcn_field_infos_t *field_infos,
                                char **field_name,
                                unsigned int n )
{
    apr_status_t s;
    lcn_field_info_t *field_info;

    LCNCR( lcn_field_infos_by_number( field_infos, &field_info, n ) );
    *field_name = (char*) field_info->name;

    return s;
}

apr_status_t
lcn_field_infos_create( lcn_field_infos_t **field_infos,
                        apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *field_infos = apr_pcalloc( pool, sizeof(lcn_field_infos_t) ), APR_ENOMEM );
        (*field_infos)->pool = pool;
    }
    while(0);

    return s;
}

apr_status_t
lcn_field_infos_field_number( lcn_field_infos_t *field_infos,
                              unsigned int *field_number,
                              const char *name )
{
    lcn_field_info_t *fi = field_infos->first_info;
    unsigned int result = 0;

    while( NULL != fi && strcmp( name, fi->name ) )
    {
        fi = fi->next;
        result++;
    }

    if ( fi == NULL )
    {
        return LCN_ERR_FIELD_NOT_FOUND;
    }

    *field_number = result;

    return APR_SUCCESS;
}

static unsigned char
lcn_convert_field_to_field_info_bits( int field_bits )
{
    unsigned char bits = 0;

    /* following field attributes do not have corresponding bits
     * in the field_info:
     *
     * LCN_FIELD_TOKENIZED
     * LCN_FIELD_BINARY
     * LCN_FIELD_COMPRESSED
     * LCN_FIELD_STORED
     */

    if ( field_bits & LCN_FIELD_INDEXED )
    {
        bits |= LCN_FIELD_INFO_IS_INDEXED;
    }

    if (field_bits & LCN_FIELD_STORE_TERM_VECTOR )
    {
        bits |= LCN_FIELD_INFO_STORE_TERM_VECTOR;
    }

    if ( field_bits & LCN_FIELD_STORE_POSITION_WITH_TV )
    {
        bits |= LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR;
    }

    if ( field_bits & LCN_FIELD_STORE_OFFSET_WITH_TV )
    {
        bits |= LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR;
    }

    if ( field_bits & LCN_FIELD_OMIT_NORMS )
    {
        bits |= LCN_FIELD_INFO_OMIT_NORMS;
    }

    if ( field_bits & LCN_FIELD_FIXED_SIZE )
    {
        bits |= LCN_FIELD_INFO_FIXED_SIZE;
    }

    return bits;
}

static int
lcn_convert_field_info_to_field_bits( unsigned char fi_bits )
{
    int bits = 0;

    /* following field attributes do not have corresponding bits
     * in the field_info:
     *
     * LCN_FIELD_TOKENIZED
     * LCN_FIELD_BINARY
     * LCN_FIELD_COMPRESSED
     * LCN_FIELD_STORED
     */

    if ( fi_bits & LCN_FIELD_INFO_IS_INDEXED )
    {
        bits |= LCN_FIELD_INDEXED;
    }

    if (fi_bits & LCN_FIELD_INFO_STORE_TERM_VECTOR )
    {
        bits |= LCN_FIELD_STORE_TERM_VECTOR;
    }

    if ( fi_bits & LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR )
    {
        bits |= LCN_FIELD_STORE_POSITION_WITH_TV;
    }

    if ( fi_bits & LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR )
    {
        bits |= LCN_FIELD_STORE_OFFSET_WITH_TV;
    }

    if ( fi_bits & LCN_FIELD_INFO_OMIT_NORMS )
    {
        bits |= LCN_FIELD_OMIT_NORMS;
    }

    if ( fi_bits & LCN_FIELD_INFO_FIXED_SIZE )
    {
        bits |= LCN_FIELD_FIXED_SIZE;
    }

    return bits;
}

static apr_status_t
lcn_field_infos_add_internal( lcn_field_infos_t *field_infos,
                              const char *name,
                              unsigned char bits )
{
    apr_status_t s;
    lcn_field_info_t *fi;

    if ( bits & LCN_FIELD_INFO_FIXED_SIZE )
    {
        return APR_SUCCESS;
    }

    do
    {
        LCNPV( fi = apr_pcalloc( field_infos->pool, sizeof(lcn_field_info_t) ), APR_ENOMEM );
        LCNPV( fi->name = lcn_atom_get_str( name ), APR_ENOMEM );

        fi->bits = bits;
        fi->field_bits = lcn_convert_field_info_to_field_bits( bits );

        /* init struct */

        if ( NULL == field_infos->last_info )
        {
            field_infos->first_info = field_infos->last_info = fi;
        }
        else
        {
            fi->number = field_infos->last_info->number + 1;
            field_infos->last_info->next = fi;
            field_infos->last_info = fi;
        }

        field_infos->size++;
    }
    while(0);

    return s;
}

/**
 * If the field is not yet known, adds it. If it is known, checks to make
 *  sure that the isIndexed flag is the same as was given previously for this
 *  field. If not - marks it as being indexed.  Same goes for storeTermVector
 *
 * @param name The name of the field
 * @param bits Field flags
 */
apr_status_t
lcn_field_infos_add_field_info( lcn_field_infos_t *field_infos,
                                const char *name,
                                unsigned char bits )
{
    apr_status_t s;
    lcn_field_info_t *fi;

    do
    {
        if ( LCN_FIELD_INFO_OMIT_NORMS & bits &&
            !( LCN_FIELD_INFO_IS_INDEXED & bits ) )
        {
            LCNCE(LCN_ERR_FIELD_INFO_OMIT_NORMS_ON_UNINDEXED);
        }

        if ( (! (LCN_FIELD_INFO_IS_INDEXED & bits)) &&
             ( LCN_FIELD_INFO_STORE_TERM_VECTOR |
               LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR |
               LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR ) & bits )
        {
            LCNCE( LCN_ERR_FIELD_INFO_STORE_TERM_VECTOR_ON_UNINDEXED );
        }

        if ( bits & (LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR |
                     LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR ) )
        {
            bits |= LCN_FIELD_INFO_STORE_TERM_VECTOR;
        }

        if ( APR_SUCCESS == (s = lcn_field_infos_by_name( field_infos, &fi, name )) )
        {
            /* the field definitions must be identical except to */
            /* the LCN_FIELD_INFO_STORE_TERM_VECTOR bit          */
            /* this differs to Java:                             */
            /* there is no "auto ajustment of field definiton"   */
            /* compare add method in FieldInfos.java             */

            if ( (fi->bits & ~LCN_FIELD_INFO_STORE_TERM_VECTOR) !=
                 (bits & ~LCN_FIELD_STORE_TERM_VECTOR ) )
            {
                LCNCE( LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );
            }

            fi->bits = bits;
            fi->field_bits = lcn_convert_field_info_to_field_bits( bits );
        }
        else if ( LCN_ERR_FIELD_NOT_FOUND == s )
        {
            LCNCM( lcn_field_infos_add_internal( field_infos, name, bits ), name );
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_field_infos_write( lcn_field_infos_t *field_infos,
                       lcn_directory_t *directory,
                       const char *file_name )
{
    apr_status_t s;
    apr_pool_t *pool;

    LCNCR( apr_pool_create( &pool, field_infos->pool ));

    do
    {
        lcn_ostream_t *ostream;

        LCNCM( lcn_directory_create_output( directory, &ostream, file_name, pool ), file_name );
        LCNCE( lcn_field_infos_write_to_ostream( field_infos, ostream ) );
        LCNCE( lcn_ostream_close( ostream ) );
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

apr_status_t
lcn_field_infos_write_to_ostream( lcn_field_infos_t *field_infos,
                                  lcn_ostream_t *ostream )
{
    apr_status_t s;

    lcn_field_info_t *fi = field_infos->first_info;

    do
    {
        LCNCE( lcn_ostream_write_vint( ostream, lcn_field_infos_size( field_infos ) ) );

        while ( NULL != fi )
        {
            unsigned char bits = fi->bits;

            LCNCE( lcn_ostream_write_string( ostream, fi->name ) );
            LCNCE( lcn_ostream_write_byte( ostream, bits ) );

            fi = fi->next;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_field_infos_add_document( lcn_field_infos_t *field_infos,
                              lcn_document_t *document )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;

    for( i = 0; i < lcn_list_size( document->field_list ); i++ )
    {
        lcn_field_t *field = lcn_list_get( document->field_list, i );

        LCNCE( lcn_field_infos_add_field_info( field_infos,
                                               field->name,
                                               lcn_convert_field_to_field_info_bits( field->flags ) ));
    }

    return s;
}


apr_status_t
lcn_field_infos_read( lcn_field_infos_t *field_infos,
                      lcn_istream_t *in,
                      apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int size, i;

        LCNCE( lcn_istream_read_vint( in, &size ) );

        for( i = 0; i < size; i++ )
        {
            char *name;
            unsigned char bits = 0;
            unsigned int len;
            unsigned int fixed_size = 0;
            char *default_value = NULL;

            LCNCE( lcn_istream_read_string( in, &name, &len, pool ) );
            LCNCE( lcn_istream_read_byte( in, &bits ) );

            if ( 0 == field_infos->format )
            {
                unsigned char new_bits = 0;

                if ( 0x1 & bits )
                {
                    new_bits |= LCN_FIELD_INFO_IS_INDEXED;
                }

                if ( 0x2 & bits )
                {
                    new_bits |= LCN_FIELD_INFO_OMIT_NORMS;
                    new_bits |= LCN_FIELD_INFO_IS_INDEXED;
                }

                bits = new_bits;
            }
            else
            {
                if ( ! (LCN_FIELD_INFO_IS_INDEXED & bits ) &&
                     (LCN_FIELD_INFO_OMIT_NORMS & bits)  )
                {
                    bits &= (~LCN_FIELD_INFO_OMIT_NORMS);
                }
            }

            if ( bits & LCN_FIELD_INFO_FIXED_SIZE )
            {
                unsigned int flen;

                LCNCE( lcn_istream_read_vint( in, &fixed_size ));

                flen = 1 + (fixed_size/8);

                LCNPV( default_value = apr_pcalloc( pool, flen ),
                       APR_ENOMEM );

                LCNCE( lcn_istream_read_bytes( in, default_value, 0, &flen ));
            }

            LCNCE( lcn_field_infos_add_field_info( field_infos, name, bits ));
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_field_infos_create_from_dir( lcn_field_infos_t **field_infos,
                                 lcn_directory_t *directory,
                                 const char *segment,
                                 apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *p;

    LCNCR( apr_pool_create( &p, pool ) );

    do
    {
        lcn_istream_t *in;
        LCNCE( lcn_directory_open_segment_file( directory, &in, segment, ".fnm", p ) );
        LCNCE( lcn_field_infos_create( field_infos, pool ));
        LCNCE( lcn_directory_segments_format( directory, &( (*field_infos)->format ) ));
        LCNCE( lcn_field_infos_read( *field_infos, in, pool ) );
        LCNCE( lcn_istream_close( in ) );
    }
    while(0);

    apr_pool_destroy( p );

    return s;
}
