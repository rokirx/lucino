#include "char_tokenizer.h"



BEGIN_C_DECLS

static lcn_bool_t
lcn_whitespace_tokenizer_is_token_char( char c )
{
    return (lcn_bool_t)!LCN_IS_WHITESPACE( c );
}

apr_status_t
lcn_whitespace_tokenizer_create( lcn_token_stream_t** token_stream,
				 const char* string,
				 apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	LCNCE( lcn_char_tokenizer_create( token_stream,
					  string,
					  pool ) );

	(*token_stream)->is_token_char = lcn_whitespace_tokenizer_is_token_char;
	
    }
    while( FALSE );

    return s;
}

END_C_DECLS

