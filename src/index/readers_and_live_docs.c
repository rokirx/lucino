#include "readers_and_live_docs.h"

apr_status_t
lcn_readers_and_live_docs_create( lcn_readers_and_live_docs_t **rld,
                                  lcn_index_writer_t *index_writer,
                                  lcn_segment_info_per_commit_t *info,
                                  apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *rld = lcn_object_create(lcn_readers_and_live_docs_t, pool ), APR_ENOMEM );

        (*rld)->info   = info;
        (*rld)->writer = index_writer;
        (*rld)->shared = LCN_TRUE;
    }
    while(0);

    return s;
}

void
lcn_readers_and_live_docs_increment( lcn_readers_and_live_docs_t *rld )
{
    rld->ref_count++;

    //TODO implement assert
    // asser rc > 1
}
