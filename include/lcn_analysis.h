#ifndef LCN_ANALYSIS_H
#define LCN_ANALYSIS_H

#include "lucene.h"

BEGIN_C_DECLS

/**
 * @defgroup analysis Analysis
 * @ingroup lucene
 * @{
 */
typedef struct lcn_token_stream_t lcn_token_stream_t;

typedef struct lcn_analyzer_private_t lcn_analyzer_private_t;

struct lcn_analyzer_t
{
    apr_status_t (*token_stream)( lcn_analyzer_t* analyzer,
                                  lcn_token_stream_t** token_stream,
                                  const char* input,
                                  apr_pool_t *pool );

    apr_pool_t* pool;

    lcn_analyzer_private_t* priv;

    const char* type;
};

/* -------------- lcn_token_t ----------------------------------------------- */



typedef int lcn_token_type_t;

/**
 * Actualized Implementation of the Analysis-Part
 */

#define LCN_TOKEN_MAX_WORD_LEN (1024)

#define LCN_TOKEN_TYPE_WORD  (0)
#define LCN_TOKEN_TYPE_DIGIT (1)



/** @brief Type to reference a lucene-token */

typedef struct lcn_token_t lcn_token_t;

struct lcn_token_t
{
    /**
     * The Text of the Term
     */
    char* term_text;

    /**
     * Start in the source Text
     */
    unsigned int start_offset;

    /**
     * End in the source Text;
     */
    unsigned int end_offset;

    /**
     * Lexical type
     */
    lcn_token_type_t type;

    unsigned int position_increment;

    /**
     * Memory-Context of the token
     */
    apr_pool_t* pool;

};

/**
 * @defgroup lcn_token Token
 * @ingroup analysis
 * @{
 */
#define INITIAL_BUFFER_CAPACITY ( 255 )
/**
 * @brief Set the position increment.  This determines the position of this token
 * relative to the previous Token in a {@link TokenStream}, used in phrase
 * searching.
 *
 * <p>The default value is one.
 *
 * <p>Some common uses for this are:<ul>
 *
 * <li>Set it to zero to put multiple terms in the same position.  This is
 * useful if, e.g., a word has multiple stems.  Searches for phrases
 * including either stem will match.  In this case, all but the first stem's
 * increment should be set to zero: the increment of the first instance
 * should be one.  Repeating a token with an increment of zero can also be
 * used to boost the scores of matches on that token.
 *
 * <li>Set it to values greater than one to inhibit exact phrase matches.
 * If, for example, one does not want phrases to match across removed stop
 * words, then one could build a stop word filter that removes stop words and
 * also sets the increment to the number of stop words removed before each
 * non-stop word.  Then exact phrase queries will only match when the terms
 * occur with no intervening stop words.
 *
 * </ul>
 */

void
lcn_token_set_position_increment( lcn_token_t* token,
                                  unsigned int position_increment );

/**
 * @brief Returns the position increment of this Token
 */

unsigned int
lcn_token_get_position_increment( lcn_token_t* token );

/**
 * @brief Returns the Token's Term-Text
 *
 * @param token Token
 * @param term_text Pointer to the Result-String
 * @param pool Memory-Context
 *
 * @result LCN-Status-Value
 */

apr_status_t
lcn_token_term_text( lcn_token_t* token,
                     char** term_text,
                     apr_pool_t* pool );

/**
 * @brief Returns this Token's starting offset, the position of the first character
 * corresponding to this token in the source text.
 *
 * Note that the difference between endOffset() and startOffset() may not be
 * equal to termText.length(), as the term text may have been altered by a
 * stemmer or some other filter.
 */

unsigned int
lcn_token_start_offset( lcn_token_t* token );

/**
 * @brief Returns this Token's ending offset, one greater than the position of the
 * last character corresponding to this token in the source text.
*/

unsigned int
lcn_token_end_offset( lcn_token_t* token );

unsigned int
lcn_token_length( lcn_token_t* token );

/**
 * @brief Returns this Token's lexical type.
 * Defaults to LCN_TOKEN_TYPE_WORD
 */

int
lcn_token_type( lcn_token_t* token );

/**
 * @brief Initializes a token with given values.
 * This is mainly for homebrew tokenizers
 */

void
lcn_token_init( lcn_token_t* token,
                const char* text,
                unsigned int start,
                unsigned int end,
                lcn_token_type_t type );

/**
 * @}
 */

/* -------------- lcn_token_stream_t ---------------------------------------- */

/**
 * @defgroup lcn_token_stream Token-Stream
 * @ingroup analysis
 * @{
 */

typedef struct lcn_token_stream_private_t lcn_token_stream_private_t;

struct lcn_token_stream_t
{
    /**
     * Retrieves the next Token from the Stream until resulting
     * LCN_TOKEN_STREAM_EOS
     *
     * @param token The next Token
     * @param token_stream Stream, to retrieve the next token from
     *
     * @result Lucene-Statusvalue
     */

    apr_status_t
    (*next)(  lcn_token_stream_t* token_stream, lcn_token_t** token );


    /**
     *
     * @brief This method is used to perform certain operations on a
     *        on characters before they are added to the token buffer.
     *        i.e. make the character lower-case
     *
     * @param char character to normalize
     * @result normalized character
     */

    char
    (*normalize)( char );

    lcn_bool_t (*is_token_char)( char character );

    lcn_bool_t  is_initialized;

    unsigned int   offset;

    char*       text;

    /**
     * Initial capacity of the Token-Text-Buffer
     */
    unsigned int  buffer_capacity;

    char*       buffer;

    apr_pool_t* pool;

    /**
     * @brief Pool, to allocate memory for the token-buffer
     */
    apr_pool_t* buffer_pool;

    /**
     * @brief Parent-Tokenstream, in case this instance was created
     *        as a filter. Tokenizers themselves don't use this.
     */

    lcn_token_stream_t* parent;

    lcn_token_stream_private_t* priv;
};

/**
 * brief Retrieves the next token in the Stream
 *
 * @param token_stream The tokenstream
 * @param token Next token in the Stream
 * @result LCN-Statusvalue. LCN_TOKEN_STREAM_EOS on end of stream
 */

apr_status_t
lcn_token_stream_next( lcn_token_stream_t* token_stream, lcn_token_t** token );


/**
 * @}
 */

/*--------------- filters ----------------------------------------  */

/**
 * @defgroup lcn_token_filter Token-Filters
 * @ingroup analysis
 * @{
 */

/**
 * @brief Normalizes token text to lower case
 *
 * @param token_filter Filtered token stream
 * @param token_stream Input stream
 * @param pool Memory context
 *
 * @result Lucene statusvalue
 */

apr_status_t
lcn_lowercase_filter_create( lcn_token_stream_t** token_filter,
                             lcn_token_stream_t* token_stream,
                             apr_pool_t* pool );


/**
 * @brief Normalizes token text by german Stemming
 */

apr_status_t
lcn_german_stem_filter_create( lcn_token_stream_t** token_filter,
                               lcn_token_stream_t* token_stream,
                               apr_pool_t* pool );

/** @} */

/*--------------- lcn_stop_filter_t ----------------------------------------  */

/**
 * @defgroup lcn_stop_filter Stop-Filter
 * @ingroup analysis
 * @brief Filter, that removes a defined set of tokens from a
 *        token-stream
 * @{
 */

typedef struct lcn_stop_filter_t lcn_stop_filter_t;


/**
 * @brief Creates a stopword-filtered stream of tokens based on a
 * lucene-list of strings
 *
 * @param stop_filter Result
 * @param token_stream The token-stream to be filtered
 * @param stop_words Lucene-list over char*
 * @param pool Memory-context
 *
 * @result Lucene-statusvalue
 */
apr_status_t
lcn_stop_filter_create_list( lcn_stop_filter_t** stop_filter,
                             lcn_token_stream_t* token_stream,
                             lcn_list_t* stop_words,
                             apr_pool_t* pool );

/**
 *  @brief Creates a stopword-filtered stream of tokens based on a
 * vector of strings
 *
 * @param stop_filter Result
 * @param token_stream The token-stream to be filtered
 * @param stop_words Zeroterminated Argument-Vector
 * @param pool Memory-context
 *
 * @result Lucene-statusvalue
 */

apr_status_t
lcn_stop_filter_create_argv( lcn_stop_filter_t** stop_filter,
                             lcn_token_stream_t* token_stream,
                             char** argv,
                             apr_pool_t* pool );


/**
 * @brief Retrieves the next Token in Stream
 *
 * @param stop_filter Filtered stream
 * @param token Next token
 *
 * @result Lucene-statusvalue LCN_TOKEN_STREAM_EOS on end of stream
 */

apr_status_t
lcn_stop_filter_next( lcn_stop_filter_t* stop_filter,
                      lcn_token_t** token );

/**
 * @}
 */

/* -------------- lcn_analyzer_t -------------------------------------------- */

/**
 * @defgroup lcn_analyzer Analyzer
 * @ingroup analysis
 * @{
 */

/**
 * @brief Retrieves an analysers token_stream.
 *
 * @param analyzer The analyzer
 * @param token_stream Result
 * @param input Text to be tokenized
 *
 * @result Lucene-statusvalue
 */

apr_status_t
lcn_analyzer_token_stream( lcn_analyzer_t* analyzer,
                           lcn_token_stream_t** token_stream,
                           const char* input,
                           apr_pool_t *pool );

/**
 * @brief Retrieves the field position gab for fields of a document
 *        using the same field name
 *
 * Invoked before indexing a Field instance if
 * terms have already been added to that field.  This allows custom
 * analyzers to place an automatic position increment gap between
 * Field instances using the same field name.  The default value
 * position increment gap is 0.  With a 0 position increment gap and
 * the typical default token position increment of 1, all terms in a field,
 * including across Field instances, are in successive positions, allowing
 * exact PhraseQuery matches, for instance, across Field instance boundaries.
 *
 * @param analyzer The analyzer
 *
 * @result position increment gap, added to the next token emitted from token stream
 */
unsigned int
lcn_analyzer_get_position_increment_gap( lcn_analyzer_t *analyzer );

const char*
lcn_analyzer_type( lcn_analyzer_t* analyzer );

/**
 * @brief Creating an analyzer that uses whitespace-tokenizer
 *
 * @param analyzer Result
 * @param pool Memory context
 *
 * @result Lucene-statusvalue
 */

apr_status_t
lcn_whitespace_analyzer_create( lcn_analyzer_t** analyzer,
                                apr_pool_t* pool );


/**
 * @brief Creating an analyzer that applies a lowercase-filter to
 *        a letter-tokenized stream
 */

apr_status_t
lcn_simple_analyzer_create( lcn_analyzer_t** analyzer,
                            apr_pool_t* pool );


apr_status_t
lcn_german_analyzer_create( lcn_analyzer_t** analyzer,
                            apr_pool_t* pool );


/* -------------- lcn_stemmer_t -------------------------------------------- */

typedef struct lcn_stemmer_t lcn_stemmer_t;


void
lcn_stemmer_stem( lcn_stemmer_t* stemmer, char* word );

apr_status_t
lcn_german_stemmer_create( lcn_stemmer_t** german_stemmer,
                           apr_pool_t* pool );

apr_status_t
lcn_analyzer_map_create( apr_hash_t** map, apr_pool_t* pool );

void
lcn_analyzer_map_add( apr_hash_t* map,
                      lcn_analyzer_t* analyzer );

/**
 * @}
 */


/**
 * @}
 */

END_C_DECLS

#endif /* LCN_ANALYSIS_H */
