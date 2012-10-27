#ifndef PHRASE_QUEUE_H
#define PHRASE_QUEUE_H

#include "lcn_util.h"

apr_status_t
lcn_phrase_queue_create( lcn_priority_queue_t** queue,
			 unsigned int capacity,
			 apr_pool_t* pool );


/**
 * @brief Queue for sloppy queries preserving term order
 *
 * @param queue     Result object
 * @param capacity  Capacity
 * @param pool      APR-Pool
 */
apr_status_t
lcn_ordered_phrase_queue_create( lcn_priority_queue_t** queue,
                                 unsigned int capacity,
                                 apr_pool_t* pool );

#endif /* PHRASE_QUEUE_H */
