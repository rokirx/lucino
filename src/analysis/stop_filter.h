#ifndef STOP_FILTER_H
#define STOP_FILTER_H

#include "lcn_analysis.h"
#include "apr_hash.h"

BEGIN_C_DECLS

struct lcn_stop_filter_t
{
    lcn_token_stream_t* input;

    apr_hash_t*         stop_words;

    apr_status_t        (*next)( lcn_stop_filter_t* stop_filter,
				 lcn_token_t** token );

    apr_pool_t*         pool;
};

			
END_C_DECLS

#endif /* STOP_FILTER_H */
