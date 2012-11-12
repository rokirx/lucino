#ifndef PHRASE_SCORER_H
#define PHRASE_SCORER_H

#include "scorer.h"

void
lcn_phrase_scorer_first_to_last( lcn_scorer_t* scorer );

void
lcn_phrase_scorer_pq_to_list( lcn_scorer_t* scorer );

void
lcn_phrase_scorer_sort( lcn_scorer_t* scorer );

apr_status_t
lcn_phrase_scorer_init( lcn_scorer_t* scorer );

apr_status_t
lcn_exact_phrase_scorer_create( lcn_scorer_t** phrase_scorer,
				lcn_weight_t* weight,
				lcn_ptr_array_t* term_positions,
				lcn_size_array_t* positions,
				lcn_similarity_t* similarity,
				lcn_byte_array_t* norms,
				apr_pool_t* pool );
apr_status_t
lcn_sloppy_phrase_scorer_create( lcn_scorer_t** phrase_scorer,
				 lcn_weight_t* weight,
				 lcn_ptr_array_t* term_positions,
				 lcn_size_array_t* positions,
				 lcn_similarity_t* similarity,
				 unsigned int slop,
				 lcn_byte_array_t* norms,
				 apr_pool_t* pool );

apr_status_t
lcn_sloppy_ordered_phrase_scorer_create( lcn_scorer_t** scorer,
                                         lcn_weight_t* weight,
                                         lcn_ptr_array_t* term_positions,
                                         lcn_size_array_t* positions,
                                         lcn_similarity_t* similarity,
                                         unsigned int slop,
                                         lcn_byte_array_t* norms,
                                         apr_pool_t* pool );


#endif /* PHRASE_SCORER_H */
