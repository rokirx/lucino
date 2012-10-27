#ifndef LETTER_TOKENIZER_H
#define LETTER_TOKENIZER_H


apr_status_t
lcn_letter_tokenizer_create( lcn_token_stream_t** token_stream,
			     const char* input,
			     apr_pool_t* pool );

#endif /* LETTER_TOKENIZER_H */
