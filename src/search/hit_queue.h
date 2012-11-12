#ifndef HIT_QUEUE_H
#define HIT_QUEUE_H

#include "lcn_util.h"
#include "top_docs.h"

apr_status_t
lcn_hit_queue_order_by( lcn_hit_queue_t *hit_queue,
                        int order_by_flag,
                        lcn_bool_t ordered_query_flag );

apr_status_t
lcn_hit_queue_order_by_bitvectors( lcn_hit_queue_t *hit_queue,
                                   lcn_list_t *bitvector_list );

lcn_bool_t
lcn_hit_queue_score_docs_compare( lcn_priority_queue_t *hit_queue,
                                  lcn_score_doc_t* a,
                                  lcn_score_doc_t* b );

#endif /* HIT_QUEUE_H */
