#ifndef TOP_DOC_COLLECTOR_H
#define TOP_DOC_COLLECTOR_H

#include "lcn_util.h"
#include "top_docs.h"
#include "hit_collector.h"
#include "hit_queue.h"

apr_status_t
lcn_top_doc_collector_create_with_hit_queue( lcn_hit_collector_t** collector,
                                             unsigned int num_hits,
                                             lcn_hit_queue_t *hit_queue,
                                             apr_pool_t *pool );

int
lcn_top_doc_collector_total_hits( lcn_hit_collector_t* collector );

apr_status_t
lcn_top_doc_collector_top_docs( lcn_hit_collector_t* collector,
                                lcn_top_docs_t** top_docs,
                                apr_pool_t* pool );

#endif /* TOP_DOC_COLLECTOR_H */
