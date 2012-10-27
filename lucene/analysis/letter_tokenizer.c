#include "char_tokenizer.h"

static lcn_bool_t
lcn_letter_tokenizer_is_token_char( char c )
{
  return LCN_IS_CHAR( c );
}

apr_status_t
lcn_letter_tokenizer_create( lcn_token_stream_t** token_stream,
			     const char* input,
			     apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	LCNCE( lcn_char_tokenizer_create( token_stream,
					  input,
					  pool ) );

	(*token_stream)->is_token_char = lcn_letter_tokenizer_is_token_char;
    }
    while( FALSE );

    return s;
}
