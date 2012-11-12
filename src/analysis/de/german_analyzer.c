#include "../analyzer.h"
#include "lcn_analysis.h"
#include "lucene.h"
#include "../letter_tokenizer.h"


BEGIN_C_DECLS

static apr_status_t
lcn_german_analyzer_token_stream( lcn_analyzer_t* analyzer,
                                  lcn_token_stream_t** token_stream,
                                  const char* input,
                                  apr_pool_t* pool )
{
    apr_status_t s;
    lcn_token_stream_t* lt;

    LCNCR( lcn_letter_tokenizer_create( &lt,
                                        input,
                                        pool ) );

    LCNCR( lcn_german_stem_filter_create( token_stream,
                                          lt,
                                          pool ) );
    return s;
}

apr_status_t
lcn_german_analyzer_create( lcn_analyzer_t** analyzer,
                            apr_pool_t* pool )
{
    apr_status_t s;
	
    do
    {
        LCNPV( (*analyzer) = apr_pcalloc( pool,
                                          sizeof( lcn_analyzer_t ) ),
               APR_ENOMEM );

        (*analyzer)->pool = pool;
        (*analyzer)->token_stream = lcn_german_analyzer_token_stream;
        (*analyzer)->type = "lcn_german_analyzer";
    }
    while( FALSE );

    return s;
}

END_C_DECLS
