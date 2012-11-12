#include "lowercase_tokenizer.h"
#include "lucene.h"


BEGIN_C_DECLS

extern unsigned char lcn_upper_to_lower_map[256];

static lcn_bool_t
lcn_lowercase_tokenizer_is_token_char( char c )
{

    return (lcn_bool_t)LCN_IS_CHAR( c );
}

static char
lcn_lowercase_tokenizer_normalize( char c )
{
    return (char)lcn_upper_to_lower_map[(unsigned char)c];
}

apr_status_t
lcn_lowercase_tokenizer_create( lcn_token_stream_t** token_stream,
				const char* input,
				apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
	LCNCE( lcn_char_tokenizer_create( token_stream,
					 input,
					 pool ) );

	(*token_stream)->normalize = lcn_lowercase_tokenizer_normalize;
	(*token_stream)->is_token_char = lcn_lowercase_tokenizer_is_token_char;
    }
    while( FALSE );

    return s;
}

END_C_DECLS
