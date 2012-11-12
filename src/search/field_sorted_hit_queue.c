#include "field_sorted_hit_queue.h"
#include "sort_field.h"


static lcn_bool_t
lcn_field_sorted_hit_queue_less_than( lcn_priority_queue_t *queue, void* a, void* b )
{
    lcn_score_doc_t *doc_a = (lcn_score_doc_t*) a;
    lcn_score_doc_t *doc_b = (lcn_score_doc_t*) b;

    /* run comparators */

    lcn_field_sorted_hit_queue_t *q = (lcn_field_sorted_hit_queue_t *) queue;
    unsigned int n = q->comparators_size;
    unsigned int i;
    int c = 0;

    for( i = 0; i < n && c == 0; i++ )
    {
        lcn_score_doc_comparator_t *cmpr = q->comparators[i];
        c = q->reverse[i] ? lcn_score_doc_comparator_compare( cmpr, doc_b, doc_a )
            : lcn_score_doc_comparator_compare( cmpr, doc_a, doc_b );
    }

    if ( 0 == c )
    {
        return doc_a->doc > doc_b->doc;
    }

    return c > 0;
}

apr_status_t
lcn_field_sorted_hit_queue_create( lcn_field_sorted_hit_queue_t ** queue,
                                   lcn_index_reader_t *index_reader,
                                   lcn_list_t *sort_fields,
                                   unsigned int size,
                                   apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_field_sorted_hit_queue_t *new_queue;
        lcn_priority_queue_t *pq;

        unsigned int n = lcn_list_size( sort_fields );
        unsigned int i;

        LCNPV( new_queue = apr_pcalloc( pool, sizeof( lcn_field_sorted_hit_queue_t)), APR_ENOMEM );

        LCNPV( new_queue->comparators = apr_pcalloc( pool, n * sizeof( lcn_score_doc_comparator_t*) ), APR_ENOMEM );

        new_queue->comparators_size = n;

        pq = lcn_cast_priority_queue( new_queue );
        pq->less_than = lcn_field_sorted_hit_queue_less_than;

        LCNCE( apr_pool_create( &(pq->pool), pool ) );

        LCNPV( new_queue->reverse = apr_pcalloc( pool, n * sizeof(int) ), APR_ENOMEM );

        for( i = 0; i < n; i++ )
        {
            lcn_score_doc_comparator_t *cmp;
            lcn_sort_field_t *sort_field = (lcn_sort_field_t*) lcn_list_get( sort_fields, i );

            switch( sort_field->type )
            {
            case LCN_SORT_FIELD_INT:
                LCNCE( lcn_score_doc_int_comparator_create( &cmp,
                                                            index_reader,
                                                            sort_field->name,
                                                            sort_field->default_int,
                                                            pool ));
                break;
            default:
                LCNCM( LCN_ERR_UNSUPPORTED_OPERATION,
                       "This type of sort_field not supported yet" );
            };

            LCNCE(s);

            new_queue->comparators[i] = cmp;
            new_queue->reverse[i] = sort_field->reverse;
        }

        LCNCE(s);

        LCNCE( lcn_priority_queue_initialize ( lcn_cast_priority_queue( new_queue ), size ));

        *queue = new_queue;
    }
    while(0);

    return s;
}
