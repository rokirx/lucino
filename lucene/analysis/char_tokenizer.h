#ifndef CHAR_TOKENIZER_H
#define CHAR_TOKENIZER_H

#include "lcn_analysis.h"


apr_status_t
lcn_char_tokenizer_create( lcn_token_stream_t** token_stream,
			   const char* string,
			   apr_pool_t* pool );
#endif
