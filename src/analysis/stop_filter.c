#include "stop_filter.h"
#include "lcn_util.h"

static apr_status_t
lcn_stop_filter_create( lcn_stop_filter_t** stop_filter,
			lcn_token_stream_t* token_stream,
			apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	apr_hash_t* stop_words_ht;


	LCNPV( *stop_filter = apr_pcalloc( pool,
					   sizeof( lcn_stop_filter_t ) ),
	       APR_ENOMEM );
	
	LCNCE( apr_pool_create( &((*stop_filter)->pool), pool ) );

	(*stop_filter)->input = token_stream;
	
	stop_words_ht = apr_hash_make( (*stop_filter)->pool );
	

	(*stop_filter)->stop_words = stop_words_ht;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_stop_filter_create_argv( lcn_stop_filter_t** stop_filter,
			     lcn_token_stream_t* token_stream,
			     char** stop_words,
			     apr_pool_t* pool )
{
   apr_status_t s;

    do
    {
	unsigned int i = 0;

	LCNPV( stop_words, LCN_ERR_NULL_PTR );

	LCNCE( lcn_stop_filter_create( stop_filter, 
				       token_stream,
				       pool ) );

	for( i=0; stop_words[i] != (char*)NULL; i++ )
	{
	    apr_hash_set( (*stop_filter)->stop_words, 
			  (const void*)stop_words[i],
			  APR_HASH_KEY_STRING,
			  (void*)1 );
	}
    }
    while( FALSE );
    
    return s;
}

apr_status_t
lcn_stop_filter_create_list( lcn_stop_filter_t** stop_filter,
			     lcn_token_stream_t* token_stream,
			     lcn_list_t* stop_words,
			     apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	unsigned int i;
	LCNPV( stop_words, LCN_ERR_NULL_PTR );

	LCNCE( lcn_stop_filter_create( stop_filter, 
				       token_stream,
				       pool ) );

	for( i = 0; i < lcn_list_size( stop_words ); i++ )
	{
	    char* act_word;
	    act_word = lcn_list_get( stop_words, i );
	    
	    apr_hash_set( (*stop_filter)->stop_words, 
			  ( const void*)act_word,
			  APR_HASH_KEY_STRING,
			  (void*)1 );
	}
    }
    while( FALSE );
    
    return s;
}

apr_status_t
lcn_stop_filter_next( lcn_stop_filter_t* stop_filter,
		      lcn_token_t** token )
{
    apr_status_t s;

    do
    {
	apr_pool_t* child_pool;
	lcn_token_stream_t* ts = stop_filter->input;
	
	LCNCE( apr_pool_create( &child_pool, stop_filter->pool ) );

	while( TRUE )
	{
	    char* act_token;
	    apr_pool_clear( child_pool );


	    if( ( APR_SUCCESS != ( s = lcn_token_stream_next( ts, token ) ) ) )
	    {
		if( s == LCN_ERR_TOKEN_STREAM_EOS )
		{
		    break;
		}

		LCNCE( s );
	    }
	    
	    LCNCE( lcn_token_term_text( *token, &act_token, child_pool ) );
	    
	    if( apr_hash_get( stop_filter->stop_words,
			      act_token,
			      APR_HASH_KEY_STRING ) != (void*)NULL )
	    {
		continue;
	    }
	    else
	    {
		break;
	    }
	}
    }
    while( FALSE );

    return s;
}
