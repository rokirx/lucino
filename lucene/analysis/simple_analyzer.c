#include "lcn_analysis.h"
#include "analyzer.h"
#include "lowercase_tokenizer.h"
#include "letter_tokenizer.h"

BEGIN_C_DECLS

static apr_status_t
lcn_simple_analyzer_token_stream( lcn_analyzer_t* analyzer,
				  lcn_token_stream_t** token_stream,
				  const char* input,
                                  apr_pool_t *pool )
{
    apr_status_t s;
    
    do
    {
	lcn_token_stream_t* lt;

	LCNCE( lcn_letter_tokenizer_create( &lt,
					    input,
					    pool ) );

	LCNCE( lcn_lowercase_filter_create( token_stream,
					    lt,
					    pool ) );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_simple_analyzer_create( lcn_analyzer_t** analyzer,
                            apr_pool_t* pool )
{
    apr_status_t s;
	
    do
    {
	LCNPV( (*analyzer) = apr_pcalloc( pool,
					  sizeof( lcn_analyzer_t ) ),
	    APR_ENOMEM );

	(*analyzer)->pool = pool;
	(*analyzer)->token_stream = lcn_simple_analyzer_token_stream;
        (*analyzer)->type         = "lcn_simple_analyzer";
    }
    while( FALSE );

    return s;
}

END_C_DECLS
