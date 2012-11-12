#include "lcn_analysis.h"
#include "char_tokenizer.h"
#include "token.h"

BEGIN_C_DECLS

static apr_status_t
lcn_char_tokenizer_double_capacity( lcn_token_stream_t* token_stream, 
				    unsigned int text_len )
{
    apr_status_t s;

    do
    {
	apr_pool_t* new_buffer_pool, *sav;
	char*       new_buffer;
	LCNCE( apr_pool_create( &new_buffer_pool,
				token_stream->pool ) );

	token_stream->buffer_capacity *= 2;

	LCNPV( new_buffer = apr_palloc( new_buffer_pool,
					token_stream->buffer_capacity ),
	       APR_ENOMEM );

	
	memcpy( new_buffer, token_stream->buffer, text_len );

	sav                       = token_stream->buffer_pool;
	token_stream->buffer      = new_buffer;
	token_stream->buffer_pool = new_buffer_pool;

	apr_pool_destroy( sav );
    }
    while( FALSE );

    return s;
}

/* For test-purposes only! is_token_char is "purely virtual" */

static lcn_bool_t
lcn_char_tokenizer_is_token_char( char c )
{
    return (lcn_bool_t) ( !(LCN_IS_WHITESPACE( c ) ) &&
			  ( ( LCN_IS_CHAR( c ) ) || ( LCN_IS_DIGIT( c ) ) ) );
}

static char
lcn_char_tokenizer_normalize( char c )
{
    return c;
}

static apr_status_t
lcn_char_tokenizer_next( lcn_token_stream_t* token_stream, 
			 lcn_token_t** token )
{
    apr_status_t s = APR_SUCCESS;
    
    do
    {
	unsigned int length = 0, start;
	char c;

	start = token_stream->offset;
	
	if( !token_stream->is_initialized )
	{
	    LCNPV( (*token) = apr_pcalloc( token_stream->pool, 
					   sizeof( lcn_token_t ) ),
		APR_ENOMEM);

	    token_stream->is_initialized = TRUE;
	}

	while( TRUE )
	{
	    if( ( c = token_stream->text[ token_stream->offset ] ) )
	    {
		token_stream->offset++;

		if( token_stream->is_token_char( c ) )
		{
		    if( length == 0 )
		    {
			start = token_stream->offset - 1;
		    }
		    
		    token_stream->buffer[ length++ ] = 
			token_stream->normalize( c );
		    
		    if( length >= ( token_stream->buffer_capacity - 1 ) )
		    {
			lcn_char_tokenizer_double_capacity( token_stream, 
							    length );
		    }
		}
		else
		{
		    if( length > 0 ) 
		    {
			break;
		    }
		}
	    }
	    else
	    {
		if( length == 0 )
		{
		    s = LCN_ERR_TOKEN_STREAM_EOS;
		}
		break;
	    }
	   
	}
	token_stream->buffer[length] = 0;

	lcn_token_init( *token,
			token_stream->buffer,
			start,
			start + length,
			LCN_TOKEN_TYPE_WORD );
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_token_stream_next( lcn_token_stream_t* token_stream,
		       lcn_token_t** token )
{
    apr_status_t s;
    if( APR_SUCCESS == ( s = token_stream->next( token_stream, token ) ) )
    {
        token_stream->is_initialized = LCN_TRUE;
    }

    return s;
}

apr_status_t
lcn_char_tokenizer_create( lcn_token_stream_t** token_stream,
			   const char* string,
			   apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	apr_pool_t* stream_pool;

	LCNCE( apr_pool_create( &stream_pool, pool ) );

	(*token_stream) = 
	    (lcn_token_stream_t*)apr_pcalloc( stream_pool,
					      sizeof( lcn_token_stream_t ) );
	(*token_stream)->pool = stream_pool;
	LCNCE( apr_pool_create( &( (*token_stream)->buffer_pool ),
				stream_pool ) );
	
	(*token_stream)->next            = lcn_char_tokenizer_next;
	(*token_stream)->is_token_char   = lcn_char_tokenizer_is_token_char;
	(*token_stream)->normalize       = lcn_char_tokenizer_normalize;

	(*token_stream)->buffer_capacity = INITIAL_BUFFER_CAPACITY;

	LCNPV( (*token_stream)->buffer = apr_palloc( (*token_stream)->buffer_pool,
						     INITIAL_BUFFER_CAPACITY ),
	       APR_ENOMEM );
	LCNPV( (*token_stream)->text = apr_pstrdup( stream_pool, string ),
	      APR_ENOMEM );
					      
	
    }
    while( FALSE );

    return s;
}

END_C_DECLS
