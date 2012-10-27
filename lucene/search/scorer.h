#ifndef SCORER_H
#define SCORER_H

#include "lcn_search.h"

#define LCN_SCORER_TYPE_BOOLEAN         ( 1 )
#define LCN_SCORER_TYPE_TERM            ( 2 )
#define LCN_SCORER_TYPE_NON_MATCHING    ( 3 )
#define LCN_SCORER_TYPE_DISJUNCTION_SUM ( 4 )
#define LCN_SCORER_TYPE_CONJUNCTION     ( 5 )
#define LCN_SCORER_TYPE_ORDERING        ( 6 )

BEGIN_C_DECLS

typedef struct lcn_coordinator_t
{
    /* to be increased for each non prohibited scorer */
    unsigned int max_coord;

    unsigned int nr_matchers;
    lcn_float_array_t* coord_factors;
    lcn_scorer_t* scorer;

    apr_pool_t* pool;

} lcn_coordinator_t;

typedef struct lcn_scorer_private_t lcn_scorer_private_t;

struct lcn_scorer_t
{
    lcn_similarity_t* similarity;
    apr_pool_t* pool;
    const char* type;
    int order;

    lcn_scorer_private_t* priv;

    apr_status_t
    (*score)( lcn_scorer_t* scorer,
              lcn_hit_collector_t* hc );

    apr_status_t
    (*score_max)( lcn_scorer_t* scorer,
                  lcn_hit_collector_t* hc,
                  unsigned int max,
                  lcn_bool_t* more_possible );

    apr_status_t
    (*next)( lcn_scorer_t* scorer );

    unsigned int
    (*doc)( lcn_scorer_t* scorer );

    apr_status_t
    (*score_get)( lcn_scorer_t* scorer,  lcn_score_t* score );

    apr_status_t
    (*skip_to)( lcn_scorer_t* scorer, unsigned int target );

};

float
lcn_coordinator_coord_factor( lcn_coordinator_t* c );

void
lcn_coordinator_init_doc( lcn_coordinator_t* coordinator );

apr_status_t
lcn_coordinator_init( lcn_coordinator_t* coordinator );

apr_status_t
lcn_coordinator_create( lcn_coordinator_t** coordinator,
                        lcn_scorer_t* scorer,
                        apr_pool_t* pool );

apr_status_t
lcn_default_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* similarity,
                           apr_pool_t* pool );

apr_status_t
lcn_ordered_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* similarity,
                           unsigned int min_nr_should_match,
                           lcn_query_type_t type,
                           apr_pool_t* pool );

apr_status_t
lcn_ordered_scorer_add( lcn_scorer_t* scorer,
                        lcn_scorer_t* to_add );

apr_status_t
lcn_conjunction_scorer_create( lcn_scorer_t** scorer,
                               lcn_similarity_t* similarity,
                               apr_pool_t* pool );

apr_status_t
lcn_conjunction_scorer_add( lcn_scorer_t* scorer, lcn_scorer_t* to_add );

apr_status_t
lcn_disjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                   lcn_list_t* sub_scorers,
                                   unsigned int minimum_nr_matchers,
                                   apr_pool_t* pool );

apr_status_t
lcn_disjunction_ordered_scorer_create( lcn_scorer_t** scorer,
                                       lcn_list_t* sub_scorers,
                                       unsigned int minimum_nr_matchers,
                                       apr_pool_t* pool );

unsigned int
lcn_disjunction_sum_score_nr_matchers( lcn_scorer_t* scorer );

apr_status_t
lcn_req_opt_sum_scorer_create( lcn_scorer_t** scorer,
                               lcn_scorer_t* req_scorer,
                               lcn_scorer_t* opt_scorer,
                               apr_pool_t* pool );

apr_status_t
lcn_req_excl_scorer_create( lcn_scorer_t** scorer,
                            lcn_scorer_t* req_scorer,
                            lcn_scorer_t* excl_scorer,
                            apr_pool_t* pool );

apr_status_t
lcn_boolean_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* similarity,
                           unsigned int min_nr_matchers,
                           lcn_query_type_t type,
                           apr_pool_t* pool );

lcn_coordinator_t*
lcn_boolean_scorer_coordinator( lcn_scorer_t* scorer );

apr_status_t
lcn_boolean_scorer_add( lcn_scorer_t* scorer,
                        lcn_scorer_t* to_add,
                        lcn_bool_t required, lcn_bool_t prohibited );
apr_status_t
lcn_non_matching_scorer_create( lcn_scorer_t** scorer, apr_pool_t* pool );

apr_status_t
lcn_counting_conjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                            lcn_scorer_t* boolean_scorer,
                                            lcn_list_t* scorers,
                                            apr_pool_t* pool );

apr_status_t
lcn_counting_disjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                            lcn_scorer_t* boolean_scorer,
                                            lcn_list_t* scorers,
                                            unsigned int min_nr_should_match,
                                            apr_pool_t* pool );
apr_status_t
lcn_single_match_scorer_create( lcn_scorer_t** single_match_scorer,
                                lcn_scorer_t* scorer,
                                lcn_coordinator_t* coordinator,
                                lcn_similarity_t* similarity,
                                apr_pool_t* pool );

apr_status_t
lcn_term_pos_scorer_create( lcn_scorer_t** scorer,
			    lcn_weight_t* weight,
			    lcn_term_docs_t* term_docs,
			    lcn_similarity_t* sim,
			    lcn_byte_array_t* norms,
			    unsigned int pos,
			    apr_pool_t* pool );

END_C_DECLS

#endif /* SCORER_H */
