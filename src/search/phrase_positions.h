#ifndef PHRASE_POSITIONS_H
#define PHRASE_POSITIONS_H

#include "lcn_search.h"

typedef lcn_priority_queue_t lcn_phrase_queue_t;
typedef struct lcn_phrase_positions_t lcn_phrase_positions_t;

struct lcn_phrase_positions_t
{
    /** current doc */
    apr_ssize_t doc;

    /** position in doc */
    apr_ssize_t position;

    /**  remaining pos in this doc */
    apr_ssize_t count;

    /** position in phrase */
    apr_ssize_t offset;

    /** stream of positions */
    lcn_term_docs_t* tp;

    /** used to make lists */
    lcn_phrase_positions_t* next;

    /* link to next repeating pp: standing for same term in different query offsets */
    lcn_phrase_positions_t* next_repeating;
};

apr_status_t
lcn_phrase_positions_skip_to( lcn_phrase_positions_t* pp, apr_ssize_t target );

apr_status_t
lcn_phrase_positions_next( lcn_phrase_positions_t* pp );

apr_status_t
lcn_phrase_positions_next_position( lcn_phrase_positions_t* pp );

apr_status_t
lcn_phrase_positions_first_position( lcn_phrase_positions_t* pp );


apr_status_t
lcn_phrase_positions_create( lcn_phrase_positions_t** phrase_positions,
			     lcn_term_docs_t* term_positions,
			     apr_ssize_t offset,
			     apr_pool_t* pool );

char*
lcn_phrase_positions_to_string( lcn_phrase_positions_t* pp,
                                apr_pool_t *pool );


#endif /* PHRASE_POSITIONS_H */
