#ifndef WHITESPACE_TOKENIZER_H
#define WHITESPACE_TOKENIZER_H

/**
 * Constructor for a Whitespace-Tokenizer.
 *
 * A Whitespace-Tokenizer is a tokenizer that divides test at whitespace
 * Adjacent sequences of non-Whitespace characters form tokens.
 *
 * @param token_stream New Token-Stream
 * @param string Input
 * @param pool Memory-Context
 *
 * @result LCN-Statusvalue
 */

apr_status_t
lcn_whitespace_tokenizer_create( lcn_token_stream_t** token_stream,
				 const char* string,
				 apr_pool_t* pool );

#endif /* WHITESPACE_TOKENIZER_H */
