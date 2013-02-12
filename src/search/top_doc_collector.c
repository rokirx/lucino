#include "top_doc_collector.h"
#include "field_sorted_hit_queue.h"
#include "index_reader.h"

#define LCN_HIT_COLLECTOR_MAX_GROUP (5)

lcn_bool_t
lcn_top_doc_collector_less_than_min( lcn_hit_collector_t *collector,
                                     unsigned int doc,
                                     lcn_score_t score )
{
    lcn_score_doc_t score_doc;
    lcn_score_doc_t min_score_doc;

    score_doc.doc = doc;
    score_doc.score = score;

    min_score_doc.doc = collector->min_doc;
    min_score_doc.score = collector->min_score;

    return lcn_hit_queue_score_docs_compare( collector->hq,
                                             &score_doc,
                                             &min_score_doc );
}

static lcn_bool_t
lcn_top_doc_collector_compare_group( lcn_hit_collector_t *collector,
                                     unsigned int doc,
                                     lcn_score_t score )
{
    lcn_score_doc_t score_doc;
    lcn_score_doc_t min_score_doc;

    score_doc.doc = doc;
    score_doc.score = score;

    min_score_doc.doc   = collector->group_min_doc;
    min_score_doc.score = collector->group_min_score;

    return lcn_hit_queue_score_docs_compare( lcn_cast_priority_queue( collector->temp_queue ),
                                             &score_doc,
                                             &min_score_doc );
}

static lcn_bool_t
is_split_group_val( unsigned int val )
{
    return LCN_FALSE;
}

static lcn_score_doc_t *
lcn_top_doc_collector_copy_element_from_removed_list( lcn_hit_collector_t *collector,
                                                      const lcn_linked_list_el_t* el )
{
    // TODO: save elementes for later use to avoid memory leak
    int k;
    lcn_score_doc_t* new_sdoc = NULL;
    lcn_score_doc_t *sdoc = lcn_linked_list_content( el );
    lcn_priority_queue_t *pq = lcn_cast_priority_queue( sdoc->group_queue );

    (void) lcn_linked_list_remove_element( collector->score_doc_removed, el );

    for( k = lcn_priority_queue_size( pq );
         k > 0 ;
         k-- )
    {
        lcn_score_doc_t *sd = lcn_priority_queue_element_at( pq, k );
        lcn_score_doc_t* score_doc = collector->score_docs[ collector->score_docs_cursor++ ];

        *score_doc = *sd;

        lcn_priority_queue_set_element_at( pq, score_doc, k );

        if ( sd == sdoc )
        {
            new_sdoc = score_doc;
        }
    }

    return new_sdoc;
}

static void
lcn_top_doc_collector_copy_element_to_removed_list( lcn_hit_collector_t *collector,
                                                    lcn_score_doc_t *removed_el )
{
    int k;
    lcn_score_doc_t *saved_el;
    lcn_priority_queue_t *pq = lcn_cast_priority_queue( removed_el->group_queue );

    for( k = lcn_priority_queue_size( pq );
         k > 0 ;
         k-- )
    {
        lcn_score_doc_t *sd = lcn_priority_queue_element_at( pq, k );
        lcn_score_doc_t *new_sd = apr_pcalloc( collector->split_group_pool, sizeof( lcn_score_doc_t ));

        *new_sd = *sd;
        collector->score_docs[ --(collector->score_docs_cursor) ] = sd;
        lcn_priority_queue_set_element_at( pq, new_sd, k );
    }

    saved_el = lcn_priority_queue_top( pq );
    *saved_el = *removed_el;

    lcn_linked_list_add_first( collector->score_doc_removed, saved_el );
}


static void
lcn_tdc_finalize_split_group_score_doc( lcn_hit_collector_t *collector,
                                        lcn_score_doc_t* removed_el )
{
    const lcn_linked_list_el_t* el =
        lcn_linked_list_first( collector->score_doc_hits );

    /* search in the list of current hits for saved element */

    while( el )
    {
        lcn_score_doc_t *sdoc = lcn_linked_list_content( el );

        if ( sdoc->group_value == removed_el->group_value )
        {
            (void) lcn_linked_list_remove_element( collector->score_doc_hits, el );
            break;
        }

        el = lcn_linked_list_next( el );
    }

    /* copy the score_doc elements in the group_queue */

    lcn_top_doc_collector_copy_element_to_removed_list( collector, removed_el );
}

static void
lcn_tdc_clear_queue( lcn_hit_collector_t *collector,
                     lcn_priority_queue_t *q )
{
    int i;

    for( i = lcn_priority_queue_size( q ); i > 0 ; i-- )
    {
        collector->score_docs[ --(collector->score_docs_cursor) ] =
            lcn_priority_queue_element_at( q, i );
    }

    lcn_priority_queue_clear( q );
    lcn_list_add( collector->queue_list, q );
}


static void
lcn_tdc_finalize_hit_group( lcn_hit_collector_t* collector )
{
    lcn_score_doc_t *score_doc = (lcn_score_doc_t*)
        lcn_priority_queue_max( lcn_cast_priority_queue( collector->temp_queue ));

    lcn_bool_t is_split_group = collector->is_split_group_val( collector->group_val );

    //nolog( stderr, "    FG:is_split_group = %d\n", is_split_group );

    score_doc->group_value = collector->group_val;

    if ( is_split_group )
    {
        //nolog( stderr, "    FG:SPLIT initialization\n" );

        if ( NULL == collector->split_group_pool )
        {
            (void) apr_pool_create( &(collector->split_group_pool), collector->pool );
            (void) lcn_linked_list_create( &(collector->score_doc_hits), collector->split_group_pool );
            (void) lcn_linked_list_create( &(collector->score_doc_removed), collector->split_group_pool );
            (void) lcn_list_create( &(collector->score_doc_list), 100, collector->split_group_pool );
        }
    }

    //nolog( stderr, "    FG:inc group_hits %d -> %d\n", collector->group_hits, collector->group_hits+1 );

    collector->group_hits++;
    score_doc->group_size = collector->temp_group_hits;

    //nolog( stderr, "    FG:set group size to %d\n", score_doc->group_size );
    collector->temp_group_hits = 0;

    if( lcn_priority_queue_size( collector->hq ) < collector->num_hits ||
        ! lcn_top_doc_collector_less_than_min( collector, score_doc->doc, score_doc->score ) )
    {
        lcn_score_doc_t *queue_top;
        lcn_score_doc_t* removed_el;

        //nolog( stderr, "    FG:insert into PQ\n" );

        /* Insert new score_doc here */

        /* save element in the list of current hits */

        if ( is_split_group )
        {
            //nolog( stderr, "    FG:SPLIT save score doc in score_doc_hits\n" );
            (void) lcn_linked_list_add_first( collector->score_doc_hits, score_doc );
        }

        removed_el = (lcn_score_doc_t*) lcn_priority_queue_insert( collector->hq, score_doc );
        score_doc->group_queue = collector->temp_queue;

        if ( NULL != removed_el )
        {
            //nolog( stderr, "    FG:handle non null removed element\n" );

            if ( NULL != removed_el->group_queue )
            {
                //nolog( stderr, "    FG:removed element has group queue\n" );

                if ( collector->is_split_group_val( removed_el->group_value ) )
                {
                    /*
                     * if the removed element contains a split group
                     * we must take care of the hits and copy them into
                     * collector->score_doc_removed
                     */
                    //nolog( stderr, "  FG:finalize split group\n" );
                    lcn_tdc_finalize_split_group_score_doc( collector, removed_el );
                }
                else
                {
                    /*
                     * take care of the queue contained in the removed element
                     */
                    //nolog( stderr, "    FG:clear removed group hit\n" );
                    lcn_tdc_clear_queue( collector, lcn_cast_priority_queue( removed_el->group_queue ));
                    removed_el->group_queue = NULL;
                }
            }
            else
            {
                //nolog( stderr, "    FG:save ungrouped hit\n" );
                collector->score_docs[ --(collector->score_docs_cursor) ] = removed_el;
            }
        }

        queue_top = (lcn_score_doc_t*) lcn_priority_queue_top( collector->hq );

        //nolog( stderr, "    FG:save min score / doc in collector\n" );

        collector->min_score = queue_top->score;
        collector->min_doc   = queue_top->doc;
    }
    else
    {
        //nolog( stderr, "    FG:score too low fr priority queue\n" );

        /* save element in the list of removed elements */

        if ( is_split_group )
        {
            //nolog( stderr, "    FG:SPLIT move to removed list\n" );
            score_doc->group_queue = collector->temp_queue;
            lcn_top_doc_collector_copy_element_to_removed_list( collector, score_doc );
        }
        else
        {
            //nolog( stderr, "    FG:clear grouped unsplitted low hit\n" );
            lcn_tdc_clear_queue( collector, lcn_cast_priority_queue( collector->temp_queue ));
        }
    }

    return;
}

static lcn_score_doc_t *
lcn_tdc_insert_new_score_doc_impl( lcn_hit_collector_t *collector,
                                   unsigned int doc,
                                   lcn_score_t score,
                                   lcn_priority_queue_t *q )
{
    lcn_score_doc_t *removed_el;
    lcn_score_doc_t *score_doc = collector->score_docs[ collector->score_docs_cursor ];

    score_doc->doc = doc;
    score_doc->score = score;

    if ((removed_el = lcn_priority_queue_insert( q, score_doc )))
    {
        collector->score_docs[ collector->score_docs_cursor ] = removed_el;
    }
    else
    {
        collector->score_docs_cursor++;
    }

    return score_doc;
}

static lcn_score_doc_t*
lcn_tdc_insert_new_score_doc( lcn_hit_collector_t *collector,
                              unsigned int doc,
                              lcn_score_t score )
{
    lcn_score_doc_t *queue_top;
    lcn_score_doc_t *score_doc = lcn_tdc_insert_new_score_doc_impl( collector,
                                                                    doc,
                                                                    score,
                                                                    collector->hq );

    queue_top = (lcn_score_doc_t*) lcn_priority_queue_top( collector->hq );

    collector->min_score = queue_top->score;
    collector->min_doc   = queue_top->doc;

    return score_doc;
}
static lcn_score_doc_t*
lcn_tdc_insert_new_score_doc_into_tmp_queue( lcn_hit_collector_t *collector,
                                             unsigned int doc,
                                             lcn_score_t score )
{
    lcn_score_doc_t *queue_top;
    lcn_score_doc_t *score_doc = lcn_tdc_insert_new_score_doc_impl( collector,
                                                                    doc,
                                                                    score,
                                                                    lcn_cast_priority_queue( collector->temp_queue ));

    queue_top = (lcn_score_doc_t*) lcn_priority_queue_top( lcn_cast_priority_queue( collector->temp_queue ));

    collector->group_min_score = queue_top->score;
    collector->group_min_doc   = queue_top->doc;

    return score_doc;
}

static void
lcn_tdc_remove_hit_from_hit_list( lcn_hit_collector_t *collector,
                                  lcn_score_doc_t *sdoc )
{
    lcn_score_doc_t *first = lcn_priority_queue_pop( collector->hq );
    lcn_score_doc_t *next;

    if ( sdoc == first )
    {
        return;
    }

    while( sdoc != ( next = lcn_priority_queue_pop( collector->hq )))
    {
        next->next = first;
        first = next;
    }

    while( first )
    {
        lcn_score_doc_t *save;

        lcn_priority_queue_insert( collector->hq, first );
        save = first->next;
        first->next = NULL;
        first = save;
    }
}


static void
lcn_tdc_start_collecting_new_group( lcn_hit_collector_t *collector,
                                    unsigned int doc,
                                    lcn_score_t score )
{
    unsigned int list_size = 0;
    lcn_score_doc_t* score_doc;
    lcn_hit_queue_t *temp_queue = NULL;

    /*
     * Check first whether it is split group with some already collected hits
     */

    //nolog( stderr, "    ON:start collecting new group %d\n", collector->group_val );

    if ( collector->is_split_group_val( collector->group_val ) &&
         NULL != collector->score_doc_hits )
    {
        const lcn_linked_list_el_t* el =
            lcn_linked_list_first( collector->score_doc_hits );

        //nolog( stderr, "    ON:is split && used score_doc_hits\n" );

        while( el )
        {
            lcn_score_doc_t *sdoc = lcn_linked_list_content( el );

            //nolog( stderr, "    ON:score_hits_iteration\n" );

            if ( sdoc->group_value == collector->group_val )
            {
                //nolog( stderr, "    ON:found group to reuse\n" );
                collector->save_score_doc = *sdoc;
                collector->temp_group_hits = sdoc->group_size;
                temp_queue = sdoc->group_queue;

                lcn_linked_list_remove_element( collector->score_doc_hits, el );
                lcn_tdc_remove_hit_from_hit_list( collector, sdoc );
                break;
            }

            el = lcn_linked_list_next( el );
        }

        if ( NULL == temp_queue && lcn_linked_list_size( collector->score_doc_removed ) > 0 )
        {
            const lcn_linked_list_el_t* el =
                lcn_linked_list_first( collector->score_doc_removed );

            //nolog( stderr, "    ON:search fr used removed list\n" );

            while( el )
            {
                lcn_score_doc_t *sdoc = lcn_linked_list_content( el );

                //nolog( stderr, "    ON:score_doc_removed iteration\n" );

                if ( sdoc->group_value == collector->group_val )
                {
                    //nolog( stderr, "    ON:found removed group to reuse\n" );
                    lcn_top_doc_collector_copy_element_from_removed_list( collector, el );

                    collector->save_score_doc = *sdoc;
                    collector->temp_group_hits = sdoc->group_size;
                    temp_queue = sdoc->group_queue;
                    break;
                }

                el = lcn_linked_list_next( el );
            }
        }
    }

    /*
     * Temp queue may be initialized from the split group hits.
     */
    if ( NULL == temp_queue )
    {
        //nolog( stderr, "    ON:temp queue is NULL\n" );

        if ( 0 == ( list_size = lcn_list_size( collector->queue_list )))
        {
            lcn_priority_queue_t *pq;

            //nolog( stderr, "    ON:create new hit queue (pool empty)o\n" );
            (void) lcn_hit_queue_create( &(collector->temp_queue),
                                         LCN_HIT_COLLECTOR_MAX_GROUP,
                                         collector->pool );

            pq = lcn_cast_priority_queue( collector->temp_queue );
            pq->less_than = collector->hq->less_than;
        }
        else
        {
            collector->temp_queue = lcn_list_get( collector->queue_list, --list_size );
            lcn_list_remove( collector->queue_list, list_size );
            //nolog( stderr, "    ON:use existing queue (new size %d)\n", list_size );
        }
    }
    else
    {
        //nolog( stderr, "    ON:decrease group_hits on reusing the score doc\n" );
        collector->temp_queue = temp_queue;
        collector->group_hits--;
    }

    /* now the collector->temp_queue is set up. collect the hit into this */

    score_doc = lcn_tdc_insert_new_score_doc_into_tmp_queue( collector, doc, score );
    score_doc->group_value = collector->group_val;
    collector->temp_group_hits++;

    //nolog( stderr, "    ON:temp group hits = %d\n", collector->temp_group_hits );
}

static void
lcn_tdc_init_custom_counts( lcn_hit_collector_t* collector,
                            unsigned int doc )
{
    (void) collector->custom_counter( collector->custom_data, doc );
}


static void
lcn_tdc_init_bitvector_counts( lcn_hit_collector_t* collector,
                               unsigned int doc )
{
    unsigned int i;
    lcn_bool_t found = LCN_FALSE;

    for( i = 0; i < collector->counting_bitvectors_size; i++ )
    {
        lcn_bitvector_t *bv = (lcn_bitvector_t*) collector->counting_bitvectors[i];

        if ( lcn_bitvector_get_bit( bv, doc ) )
        {
            collector->bv_counts[i]++;
            found = LCN_TRUE;
        }
    }

    if ( ! found )
    {
        collector->bv_counts[i]++;
    }
}

static apr_status_t
lcn_top_doc_collector_collect( lcn_hit_collector_t* collector,
                               unsigned int doc,
                               lcn_score_t score )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int group_val = 0;

    if( score.float_val <= 0.0f )
    {
        return s;
    }

    do
    {
        if( NULL != collector->bitvector &&
            ! lcn_bitvector_get_bit( collector->bitvector, doc ) )
        {
            return APR_SUCCESS;
        }

        collector->total_hits++;

        if ( NULL != collector->query_bitvector )
        {
            LCNCE( lcn_bitvector_set_bit( collector->query_bitvector, doc ));
        }

        if ( NULL != collector->boost_bitvector &&
             lcn_bitvector_get_bit( collector->boost_bitvector, doc ))
        {
            score.float_val *= collector->boost_bitvector_boost;
        }

        if ( NULL != collector->counting_bitvectors )
        {
            lcn_tdc_init_bitvector_counts( collector, doc );
        }

        if ( NULL != collector->custom_counter )
        {
            lcn_tdc_init_custom_counts( collector, doc );
        }

        /* Start collecting hit here */

        if ( NULL != collector->boost_array )
        {
            if ( 128 != collector->boost_array->arr[doc] )
            {
                score.float_val *= (collector->boost_array->arr[doc] / 128.0);
            }
        }

        /* If the group_array is not specified, then do the plain hit collecting */

        if ( NULL == collector->group_array )
        {
            collector->group_hits++;

            if( lcn_priority_queue_size( collector->hq ) < collector->num_hits ||
                ! lcn_top_doc_collector_less_than_min( collector, doc, score ) )
            {
                (void) lcn_tdc_insert_new_score_doc( collector, doc, score );
            }
        }

        /* the group_array _is_ specified. We must check the array value of the doc. */
        /* If it is 0, we must check the state of collector->group_val to possibly   */
        /* finalize the grouping of previous value                                   */

        else if ( 0 == (group_val = collector->group_array->arr[ doc ]) )
        {
            /* we score hits which must not be grouped here */

            //nolog( stderr, "\nHIT %d with 0 group_value\n", doc );

            collector->group_hits++;

            //nolog( stderr, "  TC:inc group_hits %d -> %d\n", collector->group_hits-1, collector->group_hits );

            if ( collector->group_val > 0 )
            {
                /*
                 * finalization checks, whether we have split group
                 * to save the group queues in case we need them later
                 */
                //nolog( stderr, "  TC:must finalize prev group %d\n", collector->group_val );
                lcn_tdc_finalize_hit_group( collector );
                collector->group_val = 0;
            }

            if( lcn_priority_queue_size( collector->hq ) < collector->num_hits ||
                ! lcn_top_doc_collector_less_than_min( collector, doc, score ) )
            {
                lcn_score_doc_t* score_doc =
                    lcn_tdc_insert_new_score_doc( collector, doc, score );

                //nolog( stderr, "  TC:collect 0-group hit\n" );

                score_doc->group_size = 1;
            }
        }
        else
        {
            /* The array value of the doc is positiv. We have yet to handle three cases: */
            /* 1. Continue previos grouping                                              */
            /* 2. Finalize previos grouping and start new grouping                       */
            /* 3. There is no preceding grouping. Start new grouping                     */
            /* here we must group hits for scoring */

            /* Case 1: Continue previous grouping */

            //nolog( stderr, "\nHIT %d with > group value\n", doc );

            if ( collector->group_val == group_val )
            {
                //nolog( stderr, "  TC:continue group %d\n", group_val );
                collector->temp_group_hits++;

                if (( lcn_priority_queue_size( lcn_cast_priority_queue( collector->temp_queue ))
                      < LCN_HIT_COLLECTOR_MAX_GROUP  ) ||
                    ! lcn_top_doc_collector_compare_group( collector, doc, score ))
                {
                    //nolog( stderr, "  TC:collect continued group hit\n" );
                    (void) lcn_tdc_insert_new_score_doc_into_tmp_queue( collector, doc, score );
                }
            }
            else
            {
                /* Case 2: Must finalize last grouping operation first */

                //nolog( stderr, "  TC:change to %d finalize %d first\n", group_val, collector->group_val );

                if ( collector->group_val > 0 )
                {
                    /*
                     * finalization checks, whether we have split group
                     * to save the group queues in case we need them later
                     */
                    //nolog( stderr, "  TC:start finalization of %d group\n", collector->group_val );
                    lcn_tdc_finalize_hit_group( collector );
                }

                /* Case 3: There is no preceding grouping. Start new grouping */

                /* assert that the pool of queues contains at least one priority queue */
                /* assign new priority queue and remove it from list                   */

                /*
                 * in case this group is split we must check, whether we saved
                 * the group earlier in search to continue grouping into existing
                 * score_doc
                 */
                collector->group_val = group_val;
                lcn_tdc_start_collecting_new_group( collector, doc, score );
            }
        }
    }
    while( FALSE );

    return s;
}

int
lcn_top_doc_collector_total_hits( lcn_hit_collector_t* collector )
{
    return collector->total_hits;
}

apr_status_t
lcn_top_doc_collector_top_docs( lcn_hit_collector_t* collector,
                                lcn_top_docs_t** top_docs,
                                apr_pool_t* pool  )
{
    apr_status_t s;

    do
    {
        unsigned int i, hq_size;
        lcn_ptr_array_t* score_docs;
        lcn_score_t max_score = {0};

        if ( collector->group_val > 0 )
        {
            lcn_score_doc_t *score_doc = (lcn_score_doc_t*) lcn_priority_queue_max( lcn_cast_priority_queue( collector->temp_queue ));

            collector->group_hits++;

            score_doc->group_size = collector->temp_group_hits;
            collector->temp_group_hits = 0;

            if( lcn_priority_queue_size( collector->hq ) < collector->num_hits ||
                ! lcn_top_doc_collector_less_than_min( collector, score_doc->doc, score_doc->score ) )
            {
                lcn_score_doc_t *queue_top;
                lcn_score_doc_t* removed_el = (lcn_score_doc_t*)
                    lcn_priority_queue_insert( collector->hq, score_doc );

                score_doc->group_queue = collector->temp_queue;

                if ( NULL != removed_el )
                {
                    if ( NULL != removed_el->group_queue )
                    {
                        lcn_tdc_clear_queue( collector, lcn_cast_priority_queue( removed_el->group_queue ));
                        removed_el->group_queue = NULL;
                    }
                    else
                    {
                        collector->score_docs[ --(collector->score_docs_cursor) ] = removed_el;
                    }
                }

                queue_top = (lcn_score_doc_t*) lcn_priority_queue_top( collector->hq );
                collector->min_score = queue_top->score;
                collector->min_doc   = queue_top->doc;
            }
            else
            {
                lcn_tdc_clear_queue( collector, lcn_cast_priority_queue( collector->temp_queue ));
            }
        }

        LCNCE( lcn_ptr_array_create( &score_docs,
                                     lcn_priority_queue_size( collector->hq ),
                                     pool ) );

        hq_size = lcn_priority_queue_size( collector->hq );

        for( i = 0; i < hq_size; i++ )
        {
            score_docs->arr[ hq_size - i - 1 ] =
                lcn_priority_queue_pop( collector->hq );
        }

        max_score.float_val = ( collector->total_hits == 0 )
            ? -1
            : ((lcn_score_doc_t* )score_docs->arr[0] )->score.float_val;

        LCNCE( lcn_top_docs_create( top_docs,
                                    collector->total_hits,
                                    collector->group_hits,
                                    score_docs,
                                    max_score,
                                    pool ) );

        (*top_docs)->bv_counts_size = collector->bv_counts_size;
        (*top_docs)->bv_counts      = collector->bv_counts;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_top_doc_collector_create_with_hit_queue( lcn_hit_collector_t** collector,
                                             unsigned int num_hits,
                                             lcn_hit_queue_t *hit_queue,
                                             apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        unsigned int score_docs_num;

        LCNPV( *collector = apr_pcalloc( pool,
                                         sizeof( lcn_hit_collector_t ) ),
               APR_ENOMEM );

        (*collector)->pool     = pool;
        (*collector)->num_hits = num_hits;
        (*collector)->hq       = lcn_cast_priority_queue( hit_queue );

        score_docs_num = 1 + ( num_hits * (1 + LCN_HIT_COLLECTOR_MAX_GROUP ));

        (*collector)->score_docs = (lcn_score_doc_t**)
            apr_palloc( pool, score_docs_num * sizeof(lcn_score_doc_t*) );

        for( i = 0; i < score_docs_num; i++ )
        {
            (*collector)->score_docs[i] = apr_pcalloc( pool, sizeof(lcn_score_doc_t) );
        }

        (*collector)->score_docs_cursor = 0;
        (*collector)->temp_group_hits   = 0;

        (*collector)->collect = lcn_top_doc_collector_collect;
        (*collector)->is_split_group_val = is_split_group_val;
    }
    while( FALSE );

    return s;
}
