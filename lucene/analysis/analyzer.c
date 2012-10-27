#include "analyzer.h"

BEGIN_C_DECLS

const char*
lcn_analyzer_type( lcn_analyzer_t* analyzer )
{
    return analyzer->type;
}

apr_status_t
lcn_analyzer_token_stream( lcn_analyzer_t* analyzer,
                           lcn_token_stream_t** token_stream,
                           const char* input,
                           apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
	LCNCE( analyzer->token_stream( analyzer,
		      		       token_stream,
				       input,
                                       pool ) );
    }
    while( FALSE );

    return s;
}

unsigned int
lcn_analyzer_get_position_increment_gap( lcn_analyzer_t *analyzer )
{
    return 0;
}

END_C_DECLS
