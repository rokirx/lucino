#include "lucene.h"
#include "lcn_analysis.h"
#include "lcn_util.h"

static apr_status_t
lcn_lowercase_filter_next( lcn_token_stream_t* token_filter,
                           lcn_token_t** token )
{
    apr_status_t s;
    lcn_token_stream_t* ts = token_filter->parent;

    do
    {
        if( ( s = ts->next( ts, token ) ) )
        {
            if( LCN_ERR_TOKEN_STREAM_EOS == s )
            {
                break;
            }
            LCNCE( s );
        }

        lcn_string_to_lower( (char*)(*token)->term_text );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_lowercase_filter_create( lcn_token_stream_t** token_filter,
                             lcn_token_stream_t* token_stream,
                             apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *token_filter = apr_pcalloc( pool,
                                            sizeof( lcn_token_stream_t ) ),
            APR_ENOMEM );

        LCNCE( apr_pool_create( &((*token_filter)->pool), pool ) );

        if( token_stream->parent )
        {
            (*token_filter)->parent        = token_stream->parent;
            (*token_filter)->buffer        = token_stream->parent->buffer;
            (*token_filter)->text          = token_stream->parent->text;
        }
        else
        {
            (*token_filter)->parent        = token_stream;
            (*token_filter)->buffer        = token_stream->buffer;
            (*token_filter)->text          = token_stream->text;
        }

        (*token_filter)->next          = lcn_lowercase_filter_next;
    }
    while( FALSE );

    return s;
}
