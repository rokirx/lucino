#ifndef SCORER_QUEUE_H
#define SCORER_QUEUE_H

apr_status_t
lcn_scorer_queue_create( lcn_priority_queue_t** queue,
                         unsigned int capacity,
                         apr_pool_t* pool );

#endif /* SCORER_QUEUE_H */
