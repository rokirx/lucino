#include "lcn_bitvector.h"
#include "lcn_store.h"
#include "hits.h"
#include "fs_field.h"
#include "io_context.h"

struct lcn_bitvector_t{

    apr_pool_t *pool;
    char *bits;
    unsigned int size;
    int count;

    /**
     * For lazy bitvectors: access to a bit is delegated to
     * field access.
     */
    lcn_fs_field_t *field;
    unsigned int filter_value;
};

/**
 ** Table of bits/byte
 */
static int BYTE_COUNTS[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

apr_status_t
lcn_bitvector_alloc_bits( lcn_bitvector_t *bitvector )
{
    if ( NULL == bitvector->bits )
    {
        bitvector->bits = (char *)
            apr_pcalloc( bitvector->pool, ( ( (size_t) (bitvector->size>>3) + 1 ) * sizeof(char) ) );

        if ( NULL == bitvector->bits )
        {
            return APR_ENOMEM;
        }
    }

    return APR_SUCCESS;
}

static apr_status_t
lcn_bitvector_write_internal_uncompressed( lcn_bitvector_t *bitvector,
                                           lcn_index_output_t *out )
{
    apr_status_t s;

    /* the process of writing bitvector is:
     *
     *  write size
     *  write count
     *  write bits
     */
    LCNCR( lcn_ostream_write_int( out, (int) lcn_bitvector_size( bitvector )));
    LCNCR( lcn_ostream_write_int( out, (int) lcn_bitvector_count( bitvector )));

    if ( NULL != bitvector->bits )
    {
        LCNCR( lcn_ostream_write_bytes( out,
                                        bitvector->bits,
                                        (size_t) ((bitvector->size>>3) + 1) ) );
    }
    else
    {
        unsigned int bytes = (unsigned int) ((bitvector->size >> 3) + 1 );

        while( bytes-- > 0 )
        {
            LCNCR( lcn_ostream_write_byte( out, 0 ) );
        }
    }

    return s;
}

/**
 * This function is based the following bitvector functions:
 *
 * lcn_bitvector_size
 * lcn_bitvector_count
 * lcn_bitvector_get_bit
 */
static apr_status_t
lcn_bitvector_write_internal ( lcn_bitvector_t *bv,
                               lcn_index_output_t *out )
{
    apr_status_t s;

    /* the process of writing bitvector is:
     *
     *  write version (current version is 1)
     *  write version + 1
     *  write size
     *  write count
     *  write gap coded bits
     *
     *  After reading and comparing first two integers we know,
     *  whether it is an old style file or a new style file:
     *  only in a new style file is the second integer, former count,
     *  greater then the first integer, forme size.
     */
    LCNCR( lcn_ostream_write_int( out, 1 ));
    LCNCR( lcn_ostream_write_int( out, 2 ));
    LCNCR( lcn_ostream_write_int( out, (int) lcn_bitvector_size( bv )));
    LCNCR( lcn_ostream_write_int( out, (int) lcn_bitvector_count( bv )));

    if ( NULL != bv->bits )
    {
        unsigned int i = 0;

        if ( lcn_bitvector_get_bit( bv, 0 ))
        {
            LCNCR( lcn_ostream_write_vint( out, 0 ));
        }

        while( i < lcn_bitvector_size( bv ))
        {
            lcn_bool_t bit = lcn_bitvector_get_bit( bv, i );
            unsigned int gap = 1;

            i++;

            while( (i < lcn_bitvector_size(bv)) && (bit == lcn_bitvector_get_bit( bv, i )))
            {
                gap++;
                i++;
            }

            LCNCR( lcn_ostream_write_vint( out, gap ));
        }
    }
    else
    {
        LCNCR( lcn_ostream_write_vint( out, (int) lcn_bitvector_size( bv )));
    }

    return s;
}

apr_status_t
lcn_bitvector_dump_file( lcn_bitvector_t* bitvector,
                         const char *name,
                         apr_pool_t* pool )
{
    apr_status_t s;
    lcn_index_output_t *os = NULL;
    apr_pool_t *child_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &child_pool, pool ));
        LCNCE( lcn_fs_ostream_create( &os, name, child_pool ) );
        LCNCE( lcn_bitvector_write_internal_uncompressed( bitvector, os ) );
    }
    while(0);

    if ( NULL != os )
    {
        apr_status_t stat = lcn_ostream_close( os );
        s = s ? s : stat;
    }

    if ( NULL != child_pool )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}

apr_status_t
lcn_bitvector_write_file ( lcn_bitvector_t* bitvector,
                           const char *name,
                           apr_pool_t* pool )
{
    apr_status_t s;
    lcn_index_output_t *os = NULL;
    apr_pool_t *child_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &child_pool, pool ));
        LCNCE( lcn_fs_ostream_create( &os, name, child_pool ) );
        LCNCE( lcn_bitvector_write_internal( bitvector, os ) );
    }
    while(0);

    if ( NULL != os )
    {
        apr_status_t stat = lcn_ostream_close( os );
        s = s ? s : stat;
    }

    if ( NULL != child_pool )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}

apr_status_t
lcn_bitvector_write ( lcn_bitvector_t *bitvector,
                      lcn_directory_t *dir,
                      const char *name,
                      apr_pool_t* pool )
{
    apr_status_t s;
    lcn_index_output_t *out = NULL;

    do
    {
        LCNCE( lcn_directory_create_output( dir, &out, name, pool ) );
        LCNCE( lcn_bitvector_write_internal( bitvector, out ) );
    }
    while(0);

    if ( NULL != out )
    {
        apr_status_t stat = lcn_ostream_close( out );
        s = s ? s : stat;
    }

    return s;
}

apr_status_t
lcn_bitvector_clone( lcn_bitvector_t **clone,
                     lcn_bitvector_t *bitvector,
                     apr_pool_t *pool )
{
    apr_status_t s;
    unsigned int end;

    LCNCR( lcn_null_bitvector_create( clone, bitvector->size, pool ));

    if ( NULL == bitvector->bits  && NULL == bitvector->field )
    {
        return s;
    }

    if ( NULL != bitvector->field )
    {
        (*clone)->field = bitvector->field;
        (*clone)->filter_value = bitvector->filter_value;
    }
    else
    {
        LCNCR( lcn_bitvector_alloc_bits( *clone ) );
        end = (bitvector->size >> 3) + 1;
        memcpy( (*clone)->bits, bitvector->bits, end );
    }

    (*clone)->count = bitvector->count;

    return s;
}

apr_status_t
lcn_null_bitvector_create ( lcn_bitvector_t** bitvector,
                            unsigned int size,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_bitvector_t *v;

        LCNPV( v = (lcn_bitvector_t *) apr_pcalloc( pool, sizeof(lcn_bitvector_t) ),
               APR_ENOMEM );

        v->size  = size;
        v->count = 0;
        v->pool  = pool;

        *bitvector = v;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_one_bitvector_create( lcn_bitvector_t** bitvector,
                          unsigned int size,
                          apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_bitvector_t *v;

        LCNCE( lcn_null_bitvector_create ( &v, size, pool ));
        LCNCE( lcn_bitvector_not( v ));

        *bitvector = v;
    }
    while(0);

    return s;
}

apr_status_t
lcn_bitvector_create ( lcn_bitvector_t** bitvector,
                       unsigned int size,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_null_bitvector_create( bitvector, size, pool ) );

        LCNPV( (*bitvector)->bits = (char *) apr_pcalloc( pool, ( ( (size_t) (size>>3) + 1 ) * sizeof(char) ) ),
               APR_ENOMEM );

        (*bitvector)->count = 0;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_bitvector_create_by_bits ( lcn_bitvector_t** bitvector,
                               unsigned int size,
                               char *bits,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_null_bitvector_create( bitvector, size, pool ) );
        (*bitvector)->bits = bits;
        (*bitvector)->count = -1;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_bitvector_create_by_int_fs_field ( lcn_bitvector_t** bitvector,
                                       lcn_fs_field_t *field,
                                       unsigned int filter_value,
                                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_null_bitvector_create( bitvector,
                                          lcn_fs_field_docs_count( field ),
                                          pool ) );
        (*bitvector)->field = field;
        (*bitvector)->filter_value = filter_value;
        (*bitvector)->count = -1;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_bitvector_create_from_file ( lcn_bitvector_t** bitvector,
                                 const char *file_name,
                                 apr_pool_t* pool )
{
    apr_status_t s;
    lcn_index_input_t *in = NULL;

    do
    {
        LCNCE( lcn_index_input_create( &in, file_name, pool ));
        LCNCE( lcn_bitvector_from_stream( bitvector, in, pool ) );
    }
    while(0);

    if ( NULL != in )
    {
        apr_status_t stat = lcn_index_input_close( in );
        s = s ? s : stat;
    }

    return s;
}

apr_status_t
lcn_bitvector_from_stream ( lcn_bitvector_t **bv,
                            lcn_index_input_t *in,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int len;
        int size, count;

        LCNCE(lcn_index_input_read_int( in, &size ) );

        if ( size == 0 )
        {
            LCNCE( LCN_ERR_INVALID_BV_SIZE );
        }

        LCNCE( lcn_index_input_read_int( in, &count ) );

        if ( count <= size ) /* old style file */
        {
            LCNCE( lcn_bitvector_create( bv, size, pool ) );
            (*bv)->count = count;

            /* read bits */
            len = ((*bv)->size>>3) + 1;

            if ( ( s = lcn_index_input_read_bytes( in, (*bv)->bits, 0, &len )))
            {
                /* ignore return values, as we're leaving on error */
                (void) lcn_index_input_close( in );
                break;
            }
        }
        else
        {
            int read_bits = 0;
            int bit = 0;

            LCNCE(lcn_index_input_read_int( in, &size ) );

            if ( size == 0 )
            {
                LCNCE( LCN_ERR_INVALID_BV_SIZE );
            }

            LCNCE( lcn_bitvector_create( bv, size, pool ) );
            LCNCE( lcn_index_input_read_int( in, &((*bv)->count) ) );

            while( read_bits < size )
            {
                unsigned int gap;

                LCNCE( lcn_index_input_read_vint( in, &gap ) );

                if ( 1 == bit )
                {
                    int i;

                    for( i = 0; i < gap; i++ )
                    {
                        unsigned int nth = read_bits + i;
                        (*bv)->bits[((unsigned int)nth)>>3] |= (1 << (((unsigned int)nth) & 7 ));
                    }
                }

                read_bits += gap;
                bit = 1 - bit;
            }

            (*bv)->count = -1;
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_bitvector_from_dir( lcn_bitvector_t** bitvector,
                        lcn_directory_t*  directory,
                        const char* name,
                        apr_pool_t* pool )
{
    apr_status_t s;
    lcn_index_input_t* is;

    do
    {
        LCNCE( lcn_directory_open_input( directory,
                                         &is,
                                         name,
                                         LCN_IO_CONTEXT_READONCE,
                                         pool ) );

        LCNCE( lcn_bitvector_from_stream( bitvector, is, pool ) );
    }
    while( FALSE );

    if ( NULL != is )
    {
        apr_status_t stat = lcn_index_input_close( is );
        s = s ? s : stat;
    }

    return s;
}


unsigned int
lcn_bitvector_count ( lcn_bitvector_t *bitvector )
{
    if ( NULL == bitvector->bits && NULL == bitvector->field )
    {
        return 0;
    }

    /* if the vector has been modified */

    if ( bitvector->count == -1 )
    {
        unsigned int sum = 0;
        size_t i;

        if ( NULL == bitvector->bits )
        {
            for( i = 0; i < bitvector->size; i++ )
            {
                if ( lcn_bitvector_get_bit( bitvector, i ))
                {
                    sum++;
                }
            }
        }
        else
        {
            size_t end = (bitvector->size>>3) + 1;

            for ( i = 0; i < end; i++)
            {
                /* sum bits per byte */
                sum += (unsigned int) BYTE_COUNTS[((unsigned int)(bitvector->bits[i]& 0xff ))];
            }
        }

        bitvector->count = (int) sum;
    }

    return (unsigned int) bitvector->count;
}

lcn_bool_t
lcn_bitvector_get_bit( lcn_bitvector_t* bitvector, unsigned int nth )
{
    if ( NULL == bitvector->bits )
    {
        if ( NULL == bitvector->field )
        {
            return 0;
        }
        else
        {
            unsigned int val;

            if ( APR_SUCCESS == lcn_fs_field_int_value( bitvector->field, &val, nth ))
            {
                return ( val == bitvector->filter_value );
            }
        }

        return 0;
    }

    if( nth >= bitvector->size )
    {
        LCNLOG_SIZE( "Requested bit is out of range", nth );
        LCNLOG_SIZE( "Size of Bitvector", lcn_bitvector_size( bitvector ) );
        return 0;
    }

    return ((bitvector->bits[((unsigned int)nth) >> 3] &
             (1 << (((unsigned int)nth) & 7))) != 0);
}

unsigned int
lcn_bitvector_size( const lcn_bitvector_t* bitvector )
{
    return bitvector->size;
}

char *
lcn_bitvector_bits( const lcn_bitvector_t *bitvector )
{
    return bitvector->bits;
}

lcn_bool_t
lcn_bitvector_equals( lcn_bitvector_t* a, lcn_bitvector_t* b )
{
    unsigned int i, end;

    if ( NULL != a->field || b->field )
    {
        return FALSE;
    }

    if( a->size != b->size )
    {
        return FALSE;
    }

    if ( NULL == a->bits )
    {
        if ( NULL != b->bits && 0 != lcn_bitvector_count( b ) )
        {
            return FALSE;
        }

        return TRUE;
    }

    end = (a->size >> 3) + 1;

    for( i = 0; i < end; i++ )
    {
        if( a->bits[i] != b->bits[i] )
        {
            return FALSE;
        }
    }

    return TRUE;
}

apr_status_t
lcn_bitvector_set_bit( lcn_bitvector_t* bitvector, unsigned int nth )
{
    apr_status_t s;

    LCNASSERTR( NULL == bitvector->field, LCN_ERR_UNSUPPORTED_OPERATION );

    LCNCR( lcn_bitvector_alloc_bits( bitvector ));

    bitvector->bits[((unsigned int)nth)>>3] |= (1 << (((unsigned int)nth) & 7 ));
    bitvector->count = -1;

    return s;
}

void
lcn_bitvector_clear_bit( lcn_bitvector_t* bitvector, unsigned int nth )
{
    if ( NULL == bitvector->bits && NULL == bitvector->field )
    {
        return;
    }

    bitvector->bits[((unsigned int)nth) >> 3] &= ~(1 << (((unsigned int)nth) & 7));
    bitvector->count = -1;
}

apr_status_t
lcn_bitvector_not( lcn_bitvector_t* bitvector )
{
    apr_status_t s;

    unsigned int end = (bitvector->size>>3);
    unsigned int i;
    unsigned int rest;

    LCNASSERTR( NULL == bitvector->field, LCN_ERR_UNSUPPORTED_OPERATION );

    LCNCR( lcn_bitvector_alloc_bits( bitvector ) );

    for ( i = 0; i < end; i++ )
    {
        bitvector->bits[i] = ~(bitvector->bits[i]);
    }

    if ( 0 != (rest = bitvector->size % 8) )
    {
        unsigned char mask = 255;
        mask = mask << (8-rest);
        mask = mask >> (8-rest);
        bitvector->bits[i] = ~(bitvector->bits[i]) & mask;
    }

    bitvector->count = bitvector->size - bitvector->count;

    return APR_SUCCESS;
}

apr_status_t
lcn_bitvector_uand( lcn_bitvector_t *bv_a,
                    lcn_bitvector_t *bv_b )
{
    apr_status_t s;

    LCNASSERTR( NULL == bv_a->field, LCN_ERR_UNSUPPORTED_OPERATION );
    LCNASSERTR( NULL == bv_b->field, LCN_ERR_UNSUPPORTED_OPERATION );

    do
    {
        size_t end, i;

        LCNASSERT( bv_a->size == bv_b->size, LCN_ERR_INVALID_BV_OP );

        end = (bv_a->size >> 3) + 1;

        if ( NULL == bv_b->bits )
        {
            if ( NULL != bv_a->bits )
            {
                for( i = 0; i < end; i++ )
                {
                    bv_a->bits[i] = 0;
                }
            }

            break;
        }

        LCNCE( lcn_bitvector_alloc_bits( bv_a ) );

        for ( i = 0; i < end; i++ )
        {
            bv_a->bits[i] &= bv_b->bits[i];
        }

        bv_a->count = -1;
    }
    while(0);

    return s;
}

apr_status_t
lcn_bitvector_and( lcn_bitvector_t **result,
                   lcn_bitvector_t *bv_a,
                   lcn_bitvector_t *bv_b,
                   apr_pool_t *pool )
{
    apr_status_t s;

    LCNASSERTR( NULL == bv_a->field, LCN_ERR_UNSUPPORTED_OPERATION );
    LCNASSERTR( NULL == bv_b->field, LCN_ERR_UNSUPPORTED_OPERATION );

    do
    {
        size_t end, i;

        LCNASSERT( bv_a->size == bv_b->size, LCN_ERR_INVALID_BV_OP );

        if ( NULL == bv_a->bits || NULL == bv_b->bits )
        {
            LCNCE( lcn_null_bitvector_create( result, bv_a->size, pool ));
            break;
        }

        LCNCE( lcn_bitvector_create( result, bv_a->size, pool ) );

        end = (bv_a->size >> 3) + 1;

        for ( i = 0; i < end; i++ )
        {
            (*result)->bits[i] = bv_a->bits[i] & bv_b->bits[i];
        }

        (*result)->count = -1;
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_bitvector_uor( lcn_bitvector_t *bv_a,
                   lcn_bitvector_t *bv_b )
{
    apr_status_t s;

    LCNASSERTR( NULL == bv_a->field, LCN_ERR_UNSUPPORTED_OPERATION );
    LCNASSERTR( NULL == bv_b->field, LCN_ERR_UNSUPPORTED_OPERATION );

    do
    {
        size_t end, i;

        LCNASSERT( bv_a->size == bv_b->size, LCN_ERR_INVALID_BV_OP );

        if ( NULL == bv_b->bits )
        {
            break;
        }

        LCNCE( lcn_bitvector_alloc_bits( bv_a ) );

        end = (bv_a->size >> 3) + 1;

        for ( i = 0; i < end; i++ )
        {
            bv_a->bits[i] |= bv_b->bits[i];
        }

        bv_a->count = -1;
    }
    while(0);

    return s;
}


apr_status_t
lcn_bitvector_or( lcn_bitvector_t **result,
                  lcn_bitvector_t *bv_a,
                  lcn_bitvector_t *bv_b,
                  apr_pool_t *pool )
{
    apr_status_t s;

    LCNASSERTR( NULL == bv_a->field, LCN_ERR_UNSUPPORTED_OPERATION );
    LCNASSERTR( NULL == bv_b->field, LCN_ERR_UNSUPPORTED_OPERATION );

    do
    {
        size_t end, i;

        LCNASSERT( bv_a->size == bv_b->size, LCN_ERR_INVALID_BV_OP );

        if ( NULL == bv_a->bits && NULL == bv_b->bits )
        {
            LCNCE( lcn_null_bitvector_create( result, bv_a->size, pool ));
            break;
        }

        if ( NULL == bv_a->bits )
        {
            LCNCE( lcn_bitvector_clone( result, bv_b, pool ));
            break;
        }

        if ( NULL == bv_b->bits )
        {
            LCNCE( lcn_bitvector_clone( result, bv_a, pool ));
            break;
        }

        LCNCE( lcn_bitvector_create( result, bv_a->size, pool ) );

        end = (bv_a->size >> 3) + 1;

        for ( i = 0; i < end; i++ )
        {
            (*result)->bits[i] = bv_a->bits[i] | bv_b->bits[i];
        }

        (*result)->count = -1;
    }
    while( FALSE );

    return s;
}

static int
sort_terms( const void* a, const void* b )
{
    lcn_term_t * const *ta = a;
    lcn_term_t * const *tb = b;

    int cmpres;

    if ( 0 == ( cmpres = strcmp( lcn_term_field( *ta ), lcn_term_field( *tb ) )))
    {
        return strcmp( lcn_term_text( *ta ), lcn_term_text( *tb ));
    }

    return cmpres;
}


apr_status_t
lcn_query_bitvector_create_by_term_list( lcn_bitvector_t** bitvector,
                                         lcn_list_t *term_list,
                                         lcn_searcher_t* searcher,
                                         apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    do
    {
        int i;
        lcn_index_reader_t* r = lcn_index_searcher_reader_get( searcher );

        LCNCE( apr_pool_create( &cp, pool ));
        LCNCE( lcn_list_sort( term_list, sort_terms ));
        LCNCE( lcn_bitvector_create( bitvector, lcn_index_reader_max_doc( r ), pool ));

        for( i = 0; i < lcn_list_size( term_list ); i++ )
        {
            lcn_term_t *t = lcn_list_get( term_list, i );
            lcn_term_enum_t *term_enum;

            LCNCE( lcn_index_reader_terms_from( r, &term_enum, t, cp ));

            if ( APR_SUCCESS == lcn_term_enum_next( term_enum ))
            {
                lcn_term_docs_t *td;
                apr_status_t tdnext;
                const lcn_term_t *term = lcn_term_enum_term( term_enum );

                if ( 0 == strcmp( lcn_term_field( t ), lcn_term_field( term )) &&
                     0 == strcmp( lcn_term_text( t ), lcn_term_text( term )))
                {
                    LCNCE( lcn_index_reader_term_docs_from( r, &td, term, cp ));

                    while( APR_SUCCESS == (tdnext = lcn_term_docs_next( td )))
                    {
                        lcn_bitvector_set_bit( *bitvector, lcn_term_docs_doc( td ));
                    }
                }
            }
        }
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}


apr_status_t
lcn_query_bitvector_create( lcn_bitvector_t** bitvector,
                            lcn_query_t* query,
                            lcn_searcher_t* searcher,
                            lcn_bitvector_t *search_filter,
                            apr_pool_t* pool )
{
    apr_status_t s;
    apr_pool_t* cp = NULL;

    do
    {
        lcn_hits_t* hits;
        lcn_index_reader_t* reader;

        LCNCE( apr_pool_create( &cp, pool ) );

        reader = lcn_index_searcher_reader( searcher );

        LCNCE( lcn_null_bitvector_create( bitvector,
                                          lcn_index_reader_max_doc( reader ),
                                          pool ));
        if( NULL == query )
        {
            break;
        }

        LCNCE( lcn_searcher_set_query_bitvector( searcher, *bitvector ));
        LCNCE( lcn_searcher_search( searcher,
                                    &hits,
                                    query,
                                    search_filter,
                                    cp ) );
        LCNCE( lcn_searcher_set_query_bitvector( searcher, NULL ));
    }
    while( FALSE );

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_bitvector_create_by_concat( lcn_bitvector_t **result,
                                lcn_list_t *bv_list,
                                apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        unsigned int len = 0;
        unsigned int index = 0;
        lcn_bitvector_t *first_bv;

        if ( 1 == lcn_list_size( bv_list ) )
        {
            return lcn_bitvector_clone( result,
                                        (lcn_bitvector_t*) lcn_list_get( bv_list, 0 ),
                                        pool );
        }

        for( i = 0; i < lcn_list_size( bv_list ); i++ )
        {
            lcn_bitvector_t *bv = (lcn_bitvector_t*) lcn_list_get( bv_list, i );
            LCNASSERTR( NULL == bv->field, LCN_ERR_UNSUPPORTED_OPERATION );
            len += lcn_bitvector_size( bv );
        }

        LCNCE( lcn_bitvector_create( result, len, pool ));
        LCNCE( lcn_bitvector_alloc_bits( *result ));

        first_bv = (lcn_bitvector_t*) lcn_list_get( bv_list, 0 );

        if ( NULL != first_bv->bits )
        {
            memcpy( (*result)->bits, first_bv->bits, (first_bv->size >> 3) + 1 );
        }

        index = first_bv->size;

        for( i = 1; i < lcn_list_size( bv_list ); i++ )
        {
            unsigned int j;

            lcn_bitvector_t *bv = (lcn_bitvector_t*) lcn_list_get( bv_list, i );

            if ( NULL == bv->bits )
            {
                index += bv->size;
                continue;
            }

            for ( j = 0; j < bv->size; j++, index++ )
            {
                if ( ((bv->bits[((unsigned int)j) >> 3] &
                       (1 << (((unsigned int)j) & 7))) != 0))
                {
                    (*result)->bits[((unsigned int)index)>>3] |= (1 << (((unsigned int)index) & 7 ));
                }
            }
        }

        (*result)->count = -1;
    }
    while(0);

    return s;
}

#if 0
static char*
bv_to_str( lcn_bitvector_t *bv, apr_pool_t *pool )
{
    unsigned int i;
    char *s = apr_pcalloc( pool, lcn_bitvector_size( bv ) * 2 );
    for( i = 0; i < lcn_bitvector_size( bv ); i++ )
    {
        s[lcn_bitvector_size(bv)-i-1] = ( lcn_bitvector_get_bit( bv, i )) ? '1' : '0';
    }

    s[lcn_bitvector_size(bv)] = '\0';

    return s;
}
#endif
