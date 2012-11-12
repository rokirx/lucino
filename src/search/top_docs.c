#include "lucene.h"
#include "top_docs.h"

apr_status_t
lcn_top_docs_create( lcn_top_docs_t** top_docs,
                     unsigned int total_hits,
                     unsigned int group_hits,
                     lcn_ptr_array_t* score_docs,
                     lcn_score_t max_score,
                     apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *top_docs = apr_pcalloc( pool,
                                        sizeof( lcn_top_docs_t ) ),
            APR_ENOMEM );

        (*top_docs)->total_hits = total_hits;
        (*top_docs)->group_hits = group_hits;
        (*top_docs)->max_score  = max_score;
        (*top_docs)->score_docs = score_docs;
    }
    while( FALSE );

    return s;

}

lcn_score_t
lcn_top_docs_max_score_get( lcn_top_docs_t* top_docs )
{
    return top_docs->max_score;
}

apr_status_t
lcn_score_doc_create( lcn_score_doc_t** score_doc,
                      unsigned int doc,
                      lcn_score_t score,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *score_doc = apr_pcalloc( pool, sizeof( lcn_score_doc_t ) ),
               APR_ENOMEM );

        (*score_doc)->doc   = doc;
        (*score_doc)->score = score;
    }
    while( FALSE );

    return s;
}

