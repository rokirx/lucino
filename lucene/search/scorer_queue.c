#include "lcn_util.h"
#include "scorer.h"

lcn_bool_t
lcn_scorer_queue_less_than( lcn_priority_queue_t *pq, void* o1, void* o2 )
{
    unsigned int d1 = lcn_scorer_doc( (lcn_scorer_t*)o1 );
    unsigned int d2 = lcn_scorer_doc( (lcn_scorer_t*)o2 );

    if ( d1 == d2 )
    {
        return ((lcn_scorer_t*)o1)->order < ((lcn_scorer_t*)o2)->order;
    }

    return ( d1 < d2 );
}

apr_status_t
lcn_scorer_queue_create( lcn_priority_queue_t** queue,
                         unsigned int capacity,
                         apr_pool_t* pool )
{

    apr_status_t s;

    do
    {
        LCNCE( lcn_priority_queue_create( queue,
                                          lcn_scorer_queue_less_than,
                                          pool ) );
        LCNCE( lcn_priority_queue_initialize( *queue, capacity ) );
    }
    while( FALSE );

    return s;
}
