#include "term_docs.h"

/**
 * @brief size_queue ( was IntQueue in Java )
 * @{
 */

typedef struct
{
    unsigned int arr_size;
    apr_ssize_t index;
    apr_ssize_t last_index;
    unsigned int* array;
    apr_pool_t* arr_pool;
    apr_pool_t* pool;

} lcn_mtp_size_queue_t;

static int
lcn_mtp_size_queue_cmp( const void* a, const void* b )
{
    if( *((int*)a) == *((int*)b) )
    {
        return 0;
    }
    return (  *((int*)a) < *((int*)b) ) ? -1 : 1;
}

static unsigned int
lcn_mtp_size_queue_size( lcn_mtp_size_queue_t* sq )
{
    return sq->last_index - sq->index;
}

static unsigned int
lcn_mtp_size_queue_next( lcn_mtp_size_queue_t* sq )
{
    return sq->array[sq->index++];
}

static void
lcn_mtp_size_queue_sort( lcn_mtp_size_queue_t* sq )
{
    qsort( ( sq->array + sq->index),
           lcn_mtp_size_queue_size( sq ),
           sizeof( unsigned int ),
           lcn_mtp_size_queue_cmp );
}

static void
lcn_mtp_size_queue_clear( lcn_mtp_size_queue_t* sq )
{
    sq->index = 0;
    sq->last_index = 0;
}

static apr_status_t
lcn_mtp_size_queue_grow( lcn_mtp_size_queue_t* sq )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t* new_arr_pool;
    unsigned int* new_arr;

    LCNCR( apr_pool_create( &new_arr_pool, sq->pool ) );
    LCNPR( new_arr = apr_pcalloc( new_arr_pool,
                                  ( sq->arr_size * 2 ) * sizeof( unsigned int ) ),
           APR_ENOMEM);

    memcpy( new_arr, sq->array, sq->arr_size * sizeof( unsigned int ) );
    sq->arr_size *= 2;

    sq->array = new_arr;
    apr_pool_destroy( sq->arr_pool );
    sq->arr_pool = new_arr_pool;

    return s;
}

static apr_status_t
lcn_mtp_size_queue_add( lcn_mtp_size_queue_t* sq,
                        unsigned int i )
{
    apr_status_t s = APR_SUCCESS;

    if( sq->last_index == sq->arr_size )
    {
        LCNCR( lcn_mtp_size_queue_grow( sq ) );
    }

    sq->array[ sq->last_index++ ] = i;

    return s;
}

static apr_status_t
lcn_mtp_size_queue_create( lcn_mtp_size_queue_t** sq, apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *sq = apr_pcalloc( pool, sizeof( lcn_mtp_size_queue_t ) ),
               APR_ENOMEM );

        (*sq)->arr_size   = 16;
        (*sq)->pool       = pool;

        LCNCE( apr_pool_create( &((*sq)->arr_pool), pool ) );
        LCNPV( (*sq)->array = apr_pcalloc( (*sq)->arr_pool,
                                           (*sq)->arr_size *
                                           sizeof( unsigned int ) ),
               APR_ENOMEM );
    }
    while( 0 );

    return s;
}

/** @} */

/**
 * @brief lcn_term_positions_queue
 * @{
 */

static lcn_bool_t
lcn_term_positions_queue_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    return ( lcn_term_docs_doc( (lcn_term_docs_t*)a ) <
             lcn_term_docs_doc( (lcn_term_docs_t*)b ) );
}

lcn_term_docs_t*
lcn_term_positions_queue_peek( lcn_priority_queue_t* pq )
{
    if( lcn_priority_queue_size( pq ) == 0 )
    {
        return NULL;
    }

    return lcn_priority_queue_top( pq );
}


static apr_status_t
lcn_term_positions_queue_create( lcn_priority_queue_t** pq,
                                 lcn_list_t* term_positions,
                                 apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;

        LCNCE( lcn_priority_queue_create( pq,
                                          lcn_term_positions_queue_less_than,
                                          pool ) );
        LCNCE( lcn_priority_queue_initialize(
                  *pq,
                  lcn_list_size( term_positions ) )
            );

        for( i = 0; i < lcn_list_size( term_positions ); i++ )
        {
            lcn_term_docs_t* tp;

            LCNPV( tp = lcn_list_get( term_positions, i ), LCN_ERR_NULL_PTR );

            if( APR_SUCCESS == ( s = lcn_term_docs_next( tp ) ) )
            {
                lcn_priority_queue_put( *pq, tp );
            }
            s = ( LCN_ERR_ITERATOR_NO_NEXT == s ) ? APR_SUCCESS : s;
        }
    }
    while( 0 );

    return s;
}

/** @} */

struct lcn_term_docs_private_t
{
    lcn_priority_queue_t* tp_queue;
    lcn_mtp_size_queue_t* pos_list;

    unsigned int doc;
    unsigned int freq;
};

static apr_status_t
lcn_mtp_seek_term_info ( lcn_term_docs_t *term_docs,
                         lcn_term_info_t *term_info )
{
    apr_status_t s;

    LCNCR( LCN_ERR_UNSUPPORTED_OPERATION );
}


static apr_status_t
lcn_mtp_seek_term_enum( lcn_term_docs_t *term_docs,
                        lcn_term_enum_t *term_enum )
{
    apr_status_t s;

    LCNCR( LCN_ERR_UNSUPPORTED_OPERATION );
}

static unsigned int
lcn_mtp_doc( lcn_term_docs_t* mtp )
{
    return mtp->priv->doc;
}

static unsigned int
lcn_mtp_freq( lcn_term_docs_t* mtp )
{
    return mtp->priv->freq;
}

static apr_status_t
lcn_mtp_close( lcn_term_docs_t* mtp )
{
    apr_status_t s = APR_SUCCESS;
    lcn_term_docs_private_t* p = mtp->priv;

    while( lcn_priority_queue_size( p->tp_queue ) > 0 )
    {
        lcn_term_docs_t* one_term_doc =
            lcn_priority_queue_pop( p->tp_queue );

        LCNCR( lcn_term_docs_close( one_term_doc ) );
    }

    return s;
}

static apr_status_t
lcn_mtp_next( lcn_term_docs_t* mtp )
{
    apr_status_t s;
    lcn_term_docs_private_t* p = mtp->priv;

    do
    {
        lcn_term_docs_t* tp;

        if( lcn_priority_queue_size( p->tp_queue ) == 0 )
        {
            return LCN_ERR_ITERATOR_NO_NEXT;
        }

        lcn_mtp_size_queue_clear( p->pos_list );

        p->doc = lcn_term_docs_doc( lcn_term_positions_queue_peek( p->tp_queue ) );

        do
        {
            unsigned int i;

            LCNPV( tp = lcn_term_positions_queue_peek( p->tp_queue ), LCN_ERR_NULL_PTR );

            for( i = 0; i < lcn_term_docs_freq( tp ); i++ )
            {
                apr_ssize_t next_pos;

                LCNCE( lcn_term_positions_next_position( tp, &next_pos ) );
                LCNCE( lcn_mtp_size_queue_add( p->pos_list, next_pos ) );
            }

            LCNCE( s );

            if( APR_SUCCESS == ( s = lcn_term_docs_next( tp ) ) )
            {
                lcn_priority_queue_adjust_top( p->tp_queue );
            }
            else if( s == LCN_ERR_ITERATOR_NO_NEXT )
            {
                s = APR_SUCCESS;

                lcn_priority_queue_pop( p->tp_queue );
                LCNCE( lcn_term_docs_close( tp ) );
            }
            LCNCE( s );
        }
        while( ( lcn_priority_queue_size( p->tp_queue ) > 0 ) &&
               ( lcn_term_docs_doc(
                     lcn_term_positions_queue_peek( p->tp_queue ) )
                 == p->doc ) );
        LCNCE( s );

        lcn_mtp_size_queue_sort( p->pos_list );
        p->freq = lcn_mtp_size_queue_size( p->pos_list );
    }
    while( 0 );

    return s;
}

static apr_status_t
lcn_mtp_skip_to( lcn_term_docs_t* mtp, unsigned int target )
{
    apr_status_t s;
    lcn_term_docs_private_t* p = mtp->priv;

    do
    {
        while( ( lcn_term_positions_queue_peek( p->tp_queue ) != NULL ) &&
               ( target > lcn_term_docs_doc(
                     lcn_term_positions_queue_peek( p->tp_queue ) ) ) )
        {
            lcn_term_docs_t* tp = lcn_priority_queue_pop( p->tp_queue );

            if( APR_SUCCESS == ( s = lcn_term_docs_skip_to( tp, target ) ) )
            {
                lcn_priority_queue_put( p->tp_queue, tp );
            }
            else
            {
                if( LCN_ERR_NO_SUCH_DOC == s )
                {
                    LCNCE( lcn_term_docs_close( tp ) );
                }
                else
                {
                    LCNCE( s );
                }
            }
        }
        LCNCE( lcn_mtp_next( mtp ) );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_mtp_next_position( lcn_term_docs_t* mtp, apr_ssize_t* position )
{
    *position = lcn_mtp_size_queue_next( mtp->priv->pos_list );
    return APR_SUCCESS;
}

apr_status_t
lcn_multiple_term_positions_create( lcn_term_docs_t** mtp,
                                    lcn_index_reader_t* reader,
                                    lcn_list_t* terms,
                                    apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_list_t* term_positions;
        lcn_term_docs_private_t* p;

        LCNPV( *mtp = apr_pcalloc( pool, sizeof( lcn_term_docs_t ) ),
               APR_ENOMEM );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_term_docs_private_t ) ),
               APR_ENOMEM );

        (*mtp)->priv = p;

        LCNCE( lcn_list_create( &( term_positions ), 10, pool ) );

        for( i = 0; i < lcn_list_size( terms ); i++ )
        {
            lcn_term_t* cur_term;
            lcn_term_docs_t* cur_term_positions;

            LCNPV( cur_term = lcn_list_get( terms, i ), APR_ENOMEM );

            s = lcn_index_reader_term_positions_by_term( reader,
                                                         &cur_term_positions,
                                                         cur_term,
                                                         pool );

            LCNCE( ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) ? APR_SUCCESS : s );

            LCNCE( lcn_list_add( term_positions, cur_term_positions ) );
        }

        (*mtp)->next     = lcn_mtp_next;
        (*mtp)->get_doc  = lcn_mtp_doc;
        (*mtp)->get_freq = lcn_mtp_freq;
        (*mtp)->skip_to  = lcn_mtp_skip_to;
        (*mtp)->close    = lcn_mtp_close;
        (*mtp)->next_position = lcn_mtp_next_position;
        (*mtp)->seek_term_info = lcn_mtp_seek_term_info;
        (*mtp)->seek_term_enum = lcn_mtp_seek_term_enum;

        LCNCE( lcn_term_positions_queue_create( &( p->tp_queue ),
                                                term_positions,
                                                pool ) );

        LCNCE( lcn_mtp_size_queue_create( &( p->pos_list ), pool ) );
    }
    while( FALSE );

    return s;
}
