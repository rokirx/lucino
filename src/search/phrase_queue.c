#include "phrase_queue.h"
#include "phrase_positions.h"

lcn_bool_t
lcn_ordered_phrase_queue_less_than( lcn_priority_queue_t *pq, void* a, void* b)
{
    lcn_phrase_positions_t* pp1 = a;
    lcn_phrase_positions_t* pp2 = b;

    if( pp1->doc == pp2->doc )
    {
        return ( pp1->offset == pp2->offset ) ?
            (pp1->position < pp2->position ) : (pp1->offset < pp2->offset);
    }

    return pp1->doc < pp2->doc;
}

lcn_bool_t
lcn_phrase_queue_less_than( lcn_priority_queue_t *pq, void* a, void* b)
{
    lcn_phrase_positions_t* pp1 = a;
    lcn_phrase_positions_t* pp2 = b;

    /* code from http://svn.apache.org/viewvc/lucene/java/trunk/src/java/org/apache/lucene/search/PhraseQueue.java?diff_format=h&revision=472959&view=markup&pathrev=531733 */

#if 0
    if( pp1->doc == pp2->doc )
    {
        return ( pp1->position == pp2->position ) ?
            (pp1->offset < pp2->offset) : (pp1->position < pp2->position);
    }
#endif

    /* code from http://svn.apache.org/viewvc/lucene/java/trunk/src/java/org/apache/lucene/search/PhraseQueue.java?diff_format=h&revision=531733&view=markup&pathrev=531733 */
    if (pp1->doc == pp2->doc)
    {
        if (pp1->position == pp2->position)
        {
            return pp1->offset < pp2->offset;
        }
        else
        {
            return pp1->position < pp2->position;
        }
    }

    return pp1->doc < pp2->doc;
}

apr_status_t
lcn_phrase_queue_create( lcn_priority_queue_t** queue,
                         unsigned int capacity,
                         apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_priority_queue_create( queue, lcn_phrase_queue_less_than, pool ) );
    LCNCR( lcn_priority_queue_initialize( *queue, capacity ) );

    return s;
}

apr_status_t
lcn_ordered_phrase_queue_create( lcn_priority_queue_t** queue,
                                unsigned int capacity,
                                apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_priority_queue_create( queue, lcn_ordered_phrase_queue_less_than, pool ) );
    LCNCR( lcn_priority_queue_initialize( *queue, capacity ) );

    return s;
}
