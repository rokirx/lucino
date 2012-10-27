#include "char_tokenizer.h"
#include "analyzer.h"
#include "whitespace_tokenizer.h"


BEGIN_C_DECLS

static apr_status_t
lcn_whitespace_analyzer_token_stream( lcn_analyzer_t* analyzer,
				      lcn_token_stream_t** token_stream,
				      const char* input,
                                      apr_pool_t *pool )
{
    apr_status_t s;
    
    do
    {
	LCNCE( lcn_whitespace_tokenizer_create( token_stream,
						input,
						pool ) );
						
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_whitespace_analyzer_create( lcn_analyzer_t** analyzer,
				apr_pool_t* pool )
{
    apr_status_t s;
	
    do
    {
	LCNPV( (*analyzer) = apr_pcalloc( pool,
					  sizeof( lcn_analyzer_t ) ),
	    APR_ENOMEM );

	(*analyzer)->pool = pool;
	(*analyzer)->token_stream = lcn_whitespace_analyzer_token_stream;
        (*analyzer)->type = "lcn_whitespace_analyzer";
    }
    while( FALSE );

    return s;
}

END_C_DECLS
