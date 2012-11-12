#ifndef LOWERCASE_TOKENIZER_H
#define LOWERCASE_TOKENIZER_H

#include "char_tokenizer.h"

apr_status_t
lcn_lowercase_tokenizer_create( lcn_token_stream_t**,
				const char* input,
				apr_pool_t* pool );
				
#endif /* LOWERCASE_TOKENIZER_H */
