#include "lucene.h"
#include "analysis/analyzer.h"
#include "analysis/token.h"

apr_status_t
lcn_german_stem_filter_next( lcn_token_stream_t* token_stream,
                             lcn_token_t** token )
{
    apr_status_t s;

    do
    {

        lcn_stemmer_t* stemmer;
        apr_pool_t* pool;
        lcn_token_stream_t* lcf = token_stream->parent;

        if( APR_SUCCESS != ( s = lcf->next( lcf, token ) ) )
        {
            if( s == LCN_ERR_TOKEN_STREAM_EOS )
            {
                break;
            }
            LCNCE( s );
        }

        LCNCE( apr_pool_create( &pool, token_stream->pool ) );
        LCNCE( lcn_german_stemmer_create( &stemmer,pool ) );
        lcn_stemmer_stem( stemmer, (char*)(*token)->term_text );

        apr_pool_destroy( pool );

    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_german_stem_filter_create( lcn_token_stream_t** token_filter,
                             lcn_token_stream_t* token_stream,
                             apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_token_stream_t* lcf;

        LCNCE( lcn_lowercase_filter_create( &lcf, token_stream, pool ) );

        LCNPV( *token_filter = apr_pcalloc( pool,
                                           sizeof( lcn_token_stream_t ) ),
              APR_ENOMEM );

        LCNCE( apr_pool_create( &((*token_filter)->pool), pool ) );

        (*token_filter)->parent = lcf;
        (*token_filter)->next   = lcn_german_stem_filter_next;
    }
    while( FALSE );

    return s;
}
