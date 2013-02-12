#ifndef LCN_SEARCH_H
#define LCN_SEARCH_H

#include "lucene.h"
#include "lcn_index.h"
#include "lcn_util.h"

BEGIN_C_DECLS

/**
 * @defgroup search Search
 * @ingroup lucene
 * @{
 */

typedef struct lcn_scorer_t      lcn_scorer_t;
typedef struct lcn_searcher_t    lcn_searcher_t;
typedef struct lcn_query_t       lcn_query_t;
typedef struct lcn_hits_t        lcn_hits_t;
typedef struct lcn_weight_t      lcn_weight_t;
typedef struct lcn_top_docs_t    lcn_top_docs_t;
typedef struct lcn_sort_field_t  lcn_sort_field_t;

/**
 * @defgroup score Score
 * @ingroup search
 * @{
 */

/* @} */


/**
 * @defgroup lcn_sort_field SortField
 * @ingroup search
 * @{
 */

/**
 * Consider the value of the sort field as an integer
 */
#define LCN_SORT_FIELD_INT    ( 4 )

/**
 * @brief  Sets the default int value to assume of a field in case
 *         some document does not have this field.
 *
 * @param  sort_field field to set the value on
 * @param  val        int value to be set
 */
apr_status_t
lcn_sort_field_set_default_int_value( lcn_sort_field_t *sort_field, int val );

/**
 * @brief Creates a sort field object
 *
 * @param sort_field Double pointer to store the new sort_field
 * @param field_name Field name
 * @param type       Data type of a field
 * @param reverse    Sort order
 * @param pool       APR pool
 */
apr_status_t
lcn_sort_field_create( lcn_sort_field_t** sort_field,
                       const char* field_name,
                       unsigned int type,
                       lcn_bool_t reverse,
                       apr_pool_t* pool );

/* @} */

/**
 * @defgroup query_parser lcn_query_parser_r
 * @ingroup search
 * @{
 */

typedef struct lcn_query_token_t lcn_query_token_t;
typedef struct lcn_query_info_t lcn_query_info_t;
typedef struct lcn_query_tokenizer_t lcn_query_tokenizer_t;
typedef struct lcn_hit_queue_t lcn_hit_queue_t;


/**
 * Structure holding the information needed to collect
 * and score documents.
 */
typedef struct lcn_score_doc_t lcn_score_doc_t;

typedef struct lcn_score_doc_sort_field_t {
    char type;
    union {
        int i;
        float f;
        char *s;
    } value ;
} lcn_score_doc_sort_field_t;


struct lcn_score_doc_t
{
    /**
     * Calculated score of the document
     */
    lcn_score_t score;

    /**
     * Internal document id of the document
     */
    unsigned int doc;

    /**
     * Sort fields used to sort hits by fields
     */
    lcn_score_doc_sort_field_t *fields;

    /**
     * Queue to collect group hits
     */
    lcn_hit_queue_t* group_queue;

    /**
     * Hits of the group queue
     */
    unsigned int group_size;

    /**
     * Int value to group by
     */
    unsigned int group_value;

    /**
     * pointer for temporary linked list of score docs
     */
    lcn_score_doc_t *next;
};


struct lcn_query_token_t
{
    int token_id;
    const char* text;
};


struct lcn_query_info_t
{
    apr_pool_t *pool;
    lcn_query_t *query;
    apr_status_t status;
};


/**
 * A hit queue using field sorting to collect hits
 */
struct lcn_hit_queue_t
{
    /**
     * Base class of field sorted hit queue.
     */
    lcn_priority_queue_t priority_queue;

    /**
     * Array containing bitvectors for sorting
     */
    lcn_bitvector_t **sort_bitvectors;

    /**
     * Size of sort_bitvectors array
     */
    unsigned int sort_bitvectors_size;

    /**
     * Custom data
     */
    void *data;
};


void
lcn_query_parser_parse(void*, int, lcn_query_token_t *, lcn_query_info_t* info );

void *
lcn_query_parser_alloc(void *(*malloc_proc)(size_t));

void
lcn_query_parser_trace( FILE *stream, char *z);

void lcn_query_parser_free(
    void *p,                    /* The parser to be deleted */
    void (*free_proc)(void*)     /* Function used to reclaim memory */
);

apr_status_t
lcn_query_tokenizer_create( lcn_query_tokenizer_t **tokenizer,
                            const char *text,
                            apr_pool_t *pool );

apr_status_t
lcn_query_tokenizer_next_token ( lcn_query_tokenizer_t *tokenizer,
                                 int *token_id,
                                 lcn_query_token_t *token );


/** @} */


/**
 * @defgroup occur Occur
 * @ingroup search
 * @{
 */

typedef int lcn_boolean_clause_occur_t;

/**
 * @brief Use this Operator for Terms, that MUST appear in matching
 *        documents
 */
#define LCN_BOOLEAN_CLAUSE_MUST     (0)

/**
 * @brief Use this Operator for terms, that should appear in matching
 *        documents. For a boolean query with two SHOULD subqueries, at
 *        least one of the queries must appear in the matching documents.
 */

#define LCN_BOOLEAN_CLAUSE_SHOULD   (1)

/**
 * @brief Use this operator for terms, that MUST NOT appear in the
 *        matching documents
 */

#define LCN_BOOLEAN_CLAUSE_MUST_NOT (2)

/** @} */

/**
 * @brief Structur to reference a BooleanClause
 */

typedef struct lcn_boolean_clause_t lcn_boolean_clause_t;



apr_status_t
lcn_boolean_clause_to_string( lcn_boolean_clause_t* clause,
                              char** result,
                              apr_pool_t* pool );

apr_status_t
lcn_boolean_clause_create( lcn_boolean_clause_t** clause,
                           lcn_boolean_clause_occur_t occur,
                           lcn_query_t* query,
                           apr_pool_t* pool );

lcn_bool_t
lcn_boolean_clause_equal( lcn_boolean_clause_t* a,
                          lcn_boolean_clause_t* b );

void
lcn_boolean_clause_set_query( lcn_boolean_clause_t* clause,
                              lcn_query_t* query );

lcn_query_t*
lcn_boolean_clause_query( lcn_boolean_clause_t* clause );

lcn_bool_t
lcn_boolean_clause_is_prohibited( lcn_boolean_clause_t* clause );

lcn_bool_t
lcn_boolean_clause_is_required( lcn_boolean_clause_t* clause );

lcn_boolean_clause_occur_t
lcn_boolean_clause_occur( lcn_boolean_clause_t* clause );

void
lcn_boolean_clause_set_occur( lcn_boolean_clause_t* clause,
                              lcn_boolean_clause_occur_t occur );



/**
 * @defgroup lcn_explanation_t Explanation
 * @ingroup search
 * @{
 */

typedef struct lcn_explanation_t lcn_explanation_t;

/** The value assigned to this explanation node. */

float
lcn_explanation_value_get( lcn_explanation_t* explanation );

/** Sets the value assigned to this explanation node. */

void
lcn_explanation_value_set( lcn_explanation_t* explanation, float value );

/** Sets the description of this explanation node. */

apr_status_t
lcn_explanation_description_set( lcn_explanation_t* explanation,
                                 const char* description );

/** A description of this explanation node. */

const char*
lcn_explanation_description_get( lcn_explanation_t* explanation );

/** The sub-nodes of this explanation node. */

lcn_list_t*
lcn_explanation_details_get( lcn_explanation_t* explanation );

/** Adds a sub-node to this explanation node. */

apr_status_t
lcn_explanation_detail_add( lcn_explanation_t* explanation,
                            lcn_explanation_t* detail );

apr_status_t
lcn_explanation_create( lcn_explanation_t** explanation,
                        apr_pool_t* pool );

apr_status_t
lcn_explanation_create_values( lcn_explanation_t** explanation,
                               float value,
                               const char* description,
                               apr_pool_t* pool );

apr_status_t
lcn_explanation_to_string( lcn_explanation_t* explanation,
                           char** result,
                           apr_pool_t* pool );

/** @} */

/**
 * @defgroup lcn_similarity Similarity
 * @ingroup search
 * @{
 */

apr_status_t
lcn_default_similarity_create( lcn_similarity_t** similarity,
                               apr_pool_t* pool );

float
lcn_similarity_decode_norm( const lcn_similarity_t* similarity,
                            lcn_byte_t b);

lcn_byte_t
lcn_similarity_encode_norm( const lcn_similarity_t* similarity, float f);

float
lcn_similarity_idf( const lcn_similarity_t* similarity,
                    unsigned int doc_freq,
                    unsigned int num_docs );

float
lcn_similarity_tf( const lcn_similarity_t* similarity,
                   float freq );

float
lcn_similarity_query_norm( const lcn_similarity_t* similarity,
                           float sum_of_squared_weights );

float*
lcn_similarity_norm_decoder( const lcn_similarity_t* similarity );

float
lcn_similarity_length_norm( const lcn_similarity_t* similarity,
                            const char* field_name,
                            unsigned int num_tokens );

apr_status_t
lcn_similarity_clone( const lcn_similarity_t* similarity,
                      lcn_similarity_t** new_similarity,
                      apr_pool_t* pool );

float
lcn_similarity_coord( const lcn_similarity_t* similarity,
                      unsigned int overlap,
                      unsigned int max_overlap );

/** @} */

/**
 * @defgroup lcn_hit_collector Hit-Collector
 * @ingroup search
 * @{
 */

/**
 * HitCollectors are primarily meant to be used to implement queries,
 * sorting and filtering.
 */
typedef struct lcn_hit_collector_t lcn_hit_collector_t;


/**
 * Called once for every non-zero scoring document, with the document number
 * and its score.
 */
apr_status_t
lcn_hit_collector_collect( lcn_hit_collector_t* hit_collector,
                           unsigned int doc,
                           lcn_score_t score );

/** @} */

/**
 * @defgroup lcn_weight Weight
 * @ingroup search
 * @{
 */

lcn_query_t*
lcn_weight_query( lcn_weight_t* weight );

float
lcn_weight_value( lcn_weight_t* weight );

float
lcn_weight_sum_of_squared_weights( lcn_weight_t* weight );

void
lcn_weight_normalize( lcn_weight_t* weight, float norm );

apr_status_t
lcn_weight_scorer( lcn_weight_t* weight,
                   lcn_scorer_t** scorer,
                   lcn_index_reader_t* index_reader,
                   apr_pool_t* pool );

/* Term-Weight */

apr_status_t
lcn_term_weight_create( lcn_weight_t** weight,
                        lcn_searcher_t* searcher,
                        lcn_query_t* term_query,
                        apr_pool_t* pool );
/** @} */

/**
 * @defgroup lcn_scorer Scorer
 * @ingroup search
 * @{
 */

/** Returns the Similarity implementation used by this scorer. */

lcn_similarity_t*
lcn_scorer_similarity( lcn_scorer_t* scorer );

/** Scores and collects all matching documents.
 * @param hc The collector to which all matching documents are passed through
 * {@link HitCollector#collect(int, float)}.
 * <br>When this method is used the <code>explain</code> method
 * should not be used.
 */

apr_status_t
lcn_scorer_score( lcn_scorer_t* scorer,
                  lcn_hit_collector_t* hc );

/** Expert: Collects matching documents in a range.  Hook for optimization.
 * Note that <code>next</code> must be called once before this method is called
 * for the first time.
 * @param hc The collector to which all matching documents are passed through
 * {@link HitCollector#collect(int, float)}.
 * @param max Do not score documents past this.
 * @param more_possible true if more matching documents may remain.
 */

apr_status_t
lcn_scorer_score_max( lcn_scorer_t* scorer,
                      lcn_hit_collector_t* hc,
                      unsigned int max,
                      lcn_bool_t* more_possible );

/** Advances to the next document matching the query.
 * @return true iff there is another document matching the query.
 * <br>When this method is used the <code>explain</code> method
 * should not be used.
 */

apr_status_t
lcn_scorer_next( lcn_scorer_t* scorer );

  /** Skips to the first match beyond the current whose document number is
   * greater than or equal to a given target.
   * @param target The target document number.
   * @return APR_SUCCESS if there is such a match,
   *         LCN_ERR_SCORER_NO_SUCH_DOC otherwise
   */

apr_status_t
lcn_scorer_skip_to( lcn_scorer_t* scorer,
                    unsigned int target );

/** Returns the current document number matching the query.
 * Initially invalid, until {@link #next()} is called the first time.
 */

unsigned int
lcn_scorer_doc( lcn_scorer_t* scorer );

/** Returns the current document number matching the query.
 * Initially invalid, until <code>next</code> is called the first time.
 */

apr_status_t
lcn_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score );

apr_status_t
lcn_term_scorer_create( lcn_scorer_t** scorer,
                        lcn_weight_t* weight,
                        lcn_term_docs_t* term_docs,
                        lcn_similarity_t* sim,
                        lcn_byte_array_t* norms,
                        apr_pool_t* pool );

/** @} */

/**

 * @defgroup hits Hits
 * @ingroup search
 * @{
 */

/** @} */

/**
 * @defgroup searcher Searcher
 * @ingroup search
 * @{
 */

lcn_index_reader_t*
lcn_index_searcher_reader_get( lcn_searcher_t* index_searcher );

unsigned int
lcn_searcher_max_doc( lcn_searcher_t* searcher );

apr_status_t
lcn_searcher_doc_freq( lcn_searcher_t* searcher,
                       lcn_term_t* term,
                       unsigned int* doc_freq );

void
lcn_searcher_set_hit_collector_initial_size( lcn_searcher_t *searcher,
                                             unsigned int hc_initial_size );

apr_status_t
lcn_searcher_set_boost_bitvector( lcn_searcher_t *searcher,
                                  lcn_bitvector_t *query_bitvector,
                                  double boost );

lcn_similarity_t*
lcn_searcher_similarity( lcn_searcher_t* searcher );

apr_status_t
lcn_searcher_search( lcn_searcher_t* searcher,
                     lcn_hits_t** hits,
                     lcn_query_t* query,
                     lcn_bitvector_t* bitvector,
                     apr_pool_t* pool );

apr_status_t
lcn_searcher_search_custom_hit_queue( lcn_searcher_t* searcher,
                                      lcn_hits_t** hits,
                                      lcn_query_t* query,
                                      lcn_bitvector_t* bitvector,
                                      lcn_hit_queue_t *hit_queue,
                                      apr_pool_t* pool );

apr_status_t
lcn_hit_queue_create( lcn_hit_queue_t** hit_queue,
                      unsigned int capacity,
                      apr_pool_t* pool );

apr_status_t
lcn_searcher_search_sort( lcn_searcher_t* searcher,
                          lcn_hits_t** hits,
                          lcn_query_t* query,
                          lcn_bitvector_t* bitvector,
                          lcn_list_t *sort_fields,
                          apr_pool_t* pool );

apr_status_t
lcn_searcher_get_next_doc( lcn_searcher_t* searcher,
                           lcn_document_t** new_doc,
                           lcn_document_t* document,
                           apr_pool_t *pool );


#define LCN_ORDER_BY_NATURAL   (0)
#define LCN_ORDER_BY_RELEVANCE (1)

/**
 * @brief Groups the hits by equal field values. It works correctly
 *        only if all the equal values come in consecutive order.
 *        The field must be fixed sized field of ints.
 *
 * @param searcher      Lucene Searcher
 * @param fs_field_name Name of the fixed sized field used for grouping
 */
apr_status_t
lcn_searcher_group_by( lcn_searcher_t* searcher, const char *fs_field_name );

/**
 * @brief Defines the sort strategy for hits
 *
 * @param searcher       Lucene Searcher
 * @param order_by_flag  Either LCN_ORDER_BY_RELEVANCE or LCN_ORDER_BY_NATURAL
 */
apr_status_t
lcn_searcher_order_by( lcn_searcher_t* searcher, int order_by_flag );

/**
 * @brief Sort hits by bitvectors. The hits are filtered by the
 *        bitvectors and placed in the order of the bitvectors.
 *
 * @param searcher        Lucene Searcher
 * @param bitvector_list  List of lucene bitvectors.
 */
apr_status_t
lcn_searcher_order_by_bitvectors( lcn_searcher_t *searcher,
                                  lcn_list_t *bitvector_list );

/**
 * @brief A collection of bitvectors can be passed to the
 *        searcher. For each of them a hit count is evaluated.
 *
 * @param searcher        Lucene Searcher
 * @param bitvector_list  List of lucene bitvectors.
 */
apr_status_t
lcn_searcher_set_counting_bitvectors( lcn_searcher_t *searcher,
                                      lcn_list_t *bitvector_list );

/**
 * @brief A custom collector can be passed to seacher using a
 *        callback solution
 *
 * @param searcher        Lucene Searcher
 * @param custom_counter  Callbackfunction for counting
 * @param custom_data     Data to pass to callback function
 */
apr_status_t
lcn_searcher_set_custom_counter( lcn_searcher_t *searcher,
                                 apr_status_t (*custom_counter)( void* custom_data, unsigned int doc ),
                                 void* custom_data );

/**
 * @brief The query_bitvector collects the complete resulst of the
 *        search.
 *
 * @param searcher         Lucene Searcher
 * @param query_bitvector  query_bitvector to collect hits
 */
apr_status_t
lcn_searcher_set_query_bitvector( lcn_searcher_t *searcher,
                                  lcn_bitvector_t *query_bitvector );

/**
 * @brief Sets the boost field to boost documents.
 *
 * @param searcher          Lucene Searcher
 * @param boost_field_name  Boost field name
 */
apr_status_t
lcn_searcher_set_boost_field( lcn_searcher_t *searcher,
                              const char *boost_field_name );

/**
 * @brief  Returns param function for split groups
 *
 * @param index_reader  Underlying index reader
 */
lcn_bool_t (*lcn_searcher_is_split_group_val( lcn_searcher_t *searcher ))(unsigned int);

/**
 * @brief  Sets param function for split groups
 *
 * @param index_reader  Underlying index reader
 */
void
lcn_searcher_set_split_group_func( lcn_searcher_t *searcher,
                                   lcn_bool_t (*)(unsigned int));


/**
 * @brief Returns nth docoument of the searcher
 *
 * @param searcher  Lucene Searcher
 * @param document  Result document
 * @param n         Document index
 * @param pool      APR pool
 */
apr_status_t
lcn_searcher_doc( lcn_searcher_t* searcher,
                  lcn_document_t** document,
                  unsigned int n,
                  apr_pool_t* pool );

apr_status_t
lcn_searcher_field_exists( lcn_searcher_t *searcher,
                           const char *field_name,
                           lcn_bool_t *field_exists );

lcn_bool_t
lcn_searcher_has_norms( lcn_searcher_t *searcher,
                        const char *field );

/* Index-Searcher */

/**
 * @defgroup index_searcher Index-Searcher
 * @ingroup searcher
 * @{
 */

apr_status_t
lcn_index_searcher_create_by_directory( lcn_searcher_t** index_searcher,
                                        lcn_directory_t* dir,
                                        apr_pool_t* pool );

apr_status_t
lcn_index_searcher_create_by_path( lcn_searcher_t** index_searcher,
                                   const char* path,
                                   apr_pool_t* pool );

apr_status_t
lcn_index_searcher_create_by_reader( lcn_searcher_t** index_searcher,
                                     lcn_index_reader_t* index_reader,
                                     apr_pool_t* pool );

apr_status_t
lcn_index_searcher_close( lcn_searcher_t* index_searcher );

lcn_index_reader_t*
lcn_index_searcher_reader( lcn_searcher_t* index_searcher );

apr_status_t
lcn_index_searcher_doc_freq( lcn_searcher_t* index_searcher,
                             lcn_term_t* term,
                             unsigned int* freq );

apr_status_t
lcn_index_searcher_doc( lcn_searcher_t* index_searcher,
                        lcn_document_t** document,
                        unsigned int n,
                        apr_pool_t* pool );

apr_status_t
lcn_searcher_search_top_docs( lcn_searcher_t* searcher,
                              lcn_top_docs_t** top_docs,
                              lcn_weight_t* weight,
                              lcn_bitvector_t* bitvector,
                              unsigned int n_docs );

apr_status_t
lcn_index_searcher_search_into_hc( lcn_searcher_t* index_searcher,
                                   lcn_weight_t* weight,
                                   lcn_bitvector_t* bitvector,
                                   lcn_hit_collector_t* results,
                                   apr_pool_t* pool );

apr_status_t
lcn_index_searcher_rewrite( lcn_searcher_t* searcher,
                            lcn_query_t* original,
                            lcn_query_t** rewritten,
                            apr_pool_t* pool );

/** @} */
/** @} */

/**
 * @defgroup hits Hits
 * @ingroup search
 * @{
 */



/**
 * @brief Returns the total number of hits available in this set.
 *
 * @param hits Hits
 *
 * @result Lucene-Statusvalue
 */

unsigned int
lcn_hits_length( lcn_hits_t* hits );

unsigned int
lcn_hits_total( lcn_hits_t* hits );


/**
 * @brief Returns the size of nth group in a group search
 *
 * @param hits  Hits
 * @param n     Index of the hit
 */
unsigned int
lcn_hits_group_hits_size( lcn_hits_t *hits,
                          unsigned int n );


apr_status_t
lcn_hits_doc( lcn_hits_t* hits,
              lcn_document_t** doc,
              unsigned int nth,
              apr_pool_t* pool );

/**
 * @brief Returns the document with the index 'group_index' from
 *        the nth group. The document returned by lcn_hits_doc
 *        is considered to be the 0th element of the group, i.e.
 *        the call of this function with group_index == 0 is equivalent
 *        to the call of lcn_hits_doc.
 *
 * @param hits         Hits
 * @param doc          The document to retrieve
 * @param nth          Index of the group
 * @param group_index  Index of the document in the group
 * @param pool         APR-pool
 */
apr_status_t
lcn_hits_group_doc( lcn_hits_t* hits,
                    lcn_document_t** doc,
                    unsigned int nth,
                    unsigned int group_index,
                    apr_pool_t* pool );

unsigned int
lcn_hits_bitvector_count( lcn_hits_t *hits,
                          unsigned int n );



/**
 * @brief Returns the score for the nth document in this set.
 *
 * @param hits Hits
 * @param n Number of the Document
 *
 * @result Score of the Document
 */

float
lcn_hits_score( lcn_hits_t* hits, unsigned int n );

/**
 * @brief Returns the id for the nth document in this set.
 *
 */

unsigned int
lcn_hits_id( lcn_hits_t* hits, unsigned int n );


/** @} */


/**
 * @defgroup lcn_query Query
 * @brief The abstract base class for queries
 * @ingroup search
 * @{
 */

/**
 * @defgroup query_types Query-Types
 * @ingroup lcn_query
 * @{
 */

/**
 * @brief Data-Structure to hold the type of a Lucene-Query
 */

typedef int lcn_query_type_t;


#define LCN_QUERY_TYPE_TERM           (  1 )
#define LCN_QUERY_TYPE_MULTI_TERM     (  2 )
#define LCN_QUERY_TYPE_BOOLEAN        (  3 )
#define LCN_QUERY_TYPE_WILDCARD       (  4 )
#define LCN_QUERY_TYPE_PHRASE         (  5 )
#define LCN_QUERY_TYPE_PREFIX         (  6 )
#define LCN_QUERY_TYPE_MULTIPHRASE    (  7 )
#define LCN_QUERY_TYPE_FUZZY          (  8 )
#define LCN_QUERY_TYPE_RANGE          (  9 )
#define LCN_QUERY_TYPE_SPAN           ( 10 )
#define LCN_QUERY_TYPE_DEFAULT        ( 11 )
#define LCN_QUERY_TYPE_TERM_POS       ( 12 )
#define LCN_QUERY_TYPE_MATCH_ALL_DOCS ( 13 )
#define LCN_QUERY_TYPE_ORDERED        ( 14 )
#define LCN_QUERY_TYPE_ONE_HIT        ( 15 )
#define LCN_QUERY_TYPE_FILTERED       ( 16 )


/** @} */

/** Sets the boost for this query clause to <code>b</code>.  Documents
 * matching this clause will (in addition to the normal weightings) have
 * their score multiplied by <code>b</code>.
 */

void
lcn_query_boost_set( lcn_query_t* query, float boost );

/** Gets the boost for this clause.  Documents matching
 * this clause will (in addition to the normal weightings) have their score
 * multiplied by <code>b</code>.   The boost is 1.0 by default.
 */

const char*
lcn_query_type_string( lcn_query_t* query );

float
lcn_query_boost( lcn_query_t* query );

/** Prints a query to a string. */

apr_status_t
lcn_query_to_string( lcn_query_t* query,
                     char** result,
                     const char* field,
                     apr_pool_t* pool );

/** Expert: Constructs an appropriate Weight implementation for this query.
 *
 * <p>Only implemented by primitive queries, which re-write to themselves.
 */


apr_status_t
lcn_query_create_weight( lcn_query_t* query,
                         lcn_weight_t** weight,
                         lcn_searcher_t* searcher,
                         apr_pool_t* pool );

/**
 * @brief Get name of a query
 *
 * @param query  underlying query
 */
char *
lcn_query_name( lcn_query_t *query );

/**
 * @brief Assign a name to a query
 *
 * @param query  underlying query
 * @param name   new name of the query
 */
void
lcn_query_set_name( lcn_query_t *query,
                    const char *name );

/** Expert: Constructs and initializes a Weight for a top-level query. */

apr_status_t
lcn_query_weight( lcn_query_t* query,
                  lcn_weight_t** weight,
                  lcn_searcher_t* searcher,
                  apr_pool_t* pool );

apr_status_t
lcn_query_combine( lcn_query_t** result,
                   lcn_list_t* queries,
                   apr_pool_t* pool );

apr_status_t
lcn_query_similarity( lcn_query_t* query,
                      lcn_similarity_t** similarity,
                      apr_pool_t* pool );

apr_status_t
lcn_query_extract_terms( lcn_query_t* query,
                         lcn_list_t* terms );


apr_status_t
lcn_query_clone( lcn_query_t* query, lcn_query_t** clone, apr_pool_t* pool );

lcn_query_type_t
lcn_query_type( lcn_query_t* query );

float
lcn_query_boost( lcn_query_t* query );

void
lcn_query_boost_set( lcn_query_t* query, float boost );

apr_status_t
lcn_query_rewrite( lcn_query_t* query,
                   lcn_query_t** result,
                   lcn_index_reader_t* reader,
                   apr_pool_t* pool );



/**
 * @defgroup lcn_filtered_query Filtered-Query
 * @ingroup lcn_query
 * @brief  A Query that only matches documents contained in the given filter.
 * @{
 */

apr_status_t
lcn_filtered_query_create( lcn_query_t** query,
                           lcn_bitvector_t *bv,
                           apr_pool_t* pool );

/** @} */


/**
 * @defgroup lcn_term_query Term-Query
 * @ingroup lcn_query
 * @brief  A Query that matches documents containing a term.
 *         This may be combined with other terms with a Boolean-Query.
 * @{
 */

lcn_term_t*
lcn_term_query_term( lcn_query_t* query );

apr_status_t
lcn_stop_term_query_create( lcn_query_t** query,
                            const lcn_term_t* term,
                            apr_pool_t* pool );

lcn_bool_t
lcn_term_query_is_stop_term( lcn_query_t* query );

apr_status_t
lcn_term_query_create( lcn_query_t** query,
                       const lcn_term_t* term,
                       apr_pool_t* pool );

/**
 * @brief Create query using fixed sized field for search.
 *        Currently only integer fields are supported. The
 *        term has to be of the form "fname:<number>", e.g.
 *        "id:1234"
 *
 * @param query  new query
 * @param term   term
 * @param pool   APR-Pool
 */
apr_status_t
lcn_fs_field_query_create( lcn_query_t** query,
                           const lcn_term_t* term,
                           apr_pool_t* pool );

apr_status_t
lcn_match_all_docs_query_create( lcn_query_t** query,
                                 apr_pool_t* pool );

apr_status_t
lcn_term_pos_query_create( lcn_query_t** query,
                           const lcn_term_t* term,
                           unsigned int pos,
                           apr_pool_t* pool );

apr_status_t
lcn_term_query_create_by_chars( lcn_query_t** query,
                                const char* field,
                                const char* text,
                                apr_pool_t* pool );

apr_status_t
lcn_term_pos_query_create_by_chars( lcn_query_t** query,
                                    const char* field,
                                    const char* text,
                                    unsigned int pos,
                                    apr_pool_t* pool );

/** @} */

apr_status_t
lcn_parse_query( lcn_query_t**query,
                 const char *qstring,
                 apr_pool_t *pool );


/**
 * @defgroup lcn_boolean_query Boolean-Query
 * @ingroup lcn_query
 * @brief A Query that matches documents matching boolean combinations of other
 * queries, e.g. Term-Queries or other Boolean-Queries.
 * @{
 */

/* Boolean-Query */

apr_status_t
lcn_boolean_query_create( lcn_query_t** boolean_query,
                          apr_pool_t* pool );

apr_status_t
lcn_ordered_query_create( lcn_query_t** query,
                          apr_pool_t* pool );

apr_status_t
lcn_one_hit_query_create( lcn_query_t **one_hit_query,
                          lcn_query_t *query,
                          apr_pool_t *pool );

apr_status_t
lcn_ordered_query_add( lcn_query_t* bq,
                       lcn_query_t* q );

/**
 * @brief Retrieve a list of names of the queries contained in
 *        a given ordered query. The resulting list is of size of
 *        the list of boolean clauses. An unnamed query is supposed
 *        to have an empty string as a name.
 *
 * @param query       underlying ordered query
 * @param names_list  resulting list
 * @param pool        memory context
 */
apr_status_t
lcn_ordered_query_name_list( lcn_query_t *query,
                             lcn_list_t **names_list,
                             apr_pool_t *pool );

apr_status_t
lcn_boolean_query_create_no_coord( lcn_query_t** boolean_query,
                                   apr_pool_t* pool );

apr_status_t
lcn_boolean_query_add( lcn_query_t* boolean_query,
                       lcn_query_t* query,
                       lcn_boolean_clause_occur_t occur );

apr_status_t
lcn_boolean_query_add_term( lcn_query_t* boolean_query,
                            const char* field,
                            const char* text,
                            lcn_boolean_clause_occur_t occur );

lcn_list_t*
lcn_boolean_query_clauses( lcn_query_t* boolean_query );

unsigned int
lcn_boolean_query_minimum_nr_should_match( lcn_query_t* query );

void
lcn_boolean_query_minimum_nr_should_match_set( lcn_query_t* query,
                                               unsigned int min );

/** @} */

/**
 * @defgroup lcn_multi_phrase_query MultiPhrase-Query
 * @ingroup lcn_query
 * @brief  MultiPhraseQuery is a generalized version of PhraseQuery, with an added method:
 * <PRE>
 *         lcn_multi_phrase_query_add_terms( lcn_query_t* query,
 *                                           const lcn_list_t* terms )
 * </PRE>
 *      To use this class, to search for the phrase "Microsoft app*" first
 *      use
 * <PRE>
 *     lcn_multi_phrase_query_add( lcn_query_t* query,
 *                                 lcn_term_t* term )
 * </PRE> on the term "Microsoft", then find all terms that
 *      have "app" as prefix using
 * <PRE> lcn_index_reader_terms_from( lcn_index_reader_t* reader,
 *                                    lcn_term_enum_t** term_enum,
 *                                    lcn_term_t* term,
 *                                    apr_pool_t* pool )
 * </PRE>
 * MultiPhraseQuery.add(Term[] * terms) to add them to the query.
 * @{
 */

apr_status_t
lcn_multi_phrase_query_create( lcn_query_t** query,
                               apr_pool_t* pool );

/** Sets the phrase slop for this query. */

apr_status_t
lcn_multi_phrase_query_slop_set( lcn_query_t* query,
                                 unsigned int slop );

apr_status_t
lcn_multi_phrase_query_get_field_name( lcn_query_t* query,
                                       const char **field_name );


/** Sets the phrase slop for this query. */

apr_status_t
lcn_multi_phrase_query_slop( lcn_query_t* query, unsigned int *slop );


/** Sets the phrase slop for this query. */

apr_status_t
lcn_multi_phrase_query_min_slop_set( lcn_query_t* query,
                                     unsigned int min_slop );

/** Sets the phrase slop for this query. */

apr_status_t
lcn_multi_phrase_query_min_slop( lcn_query_t* query, unsigned int *slop );


apr_status_t
lcn_multi_phrase_query_preserve_term_order( lcn_query_t* query, lcn_bool_t *preserve_term_order );


/**
 * @brief Preserve term order in a sloppy query
 *
 * @param query        Query-Object
 * @param do_preserve  If LCN_TRUE, the order of terms is preserved. Default
 *                     is LCN_FALSE.
 */
apr_status_t
lcn_multi_phrase_query_preserve_term_order_set( lcn_query_t* query,
                                                lcn_bool_t do_preserve );


/** Add a single term at the next position in the phrase */

apr_status_t
lcn_multi_phrase_query_add_term( lcn_query_t* query,
                                 const lcn_term_t* term );

/** Add multiple terms at the next position in the phrase.  Any of the terms
 * may match.
 */

apr_status_t
lcn_multi_phrase_query_add_terms( lcn_query_t* query, const lcn_list_t* terms );

apr_status_t
lcn_multi_phrase_query_add_terms_at( lcn_query_t* query,
                                     const lcn_list_t* terms,
                                     unsigned int position );

apr_status_t
lcn_multi_phrase_query_positions( lcn_query_t* query,
                                  lcn_size_array_t** positions,
                                  apr_pool_t* pool );

apr_status_t
lcn_multi_phrase_query_term_arrays( lcn_query_t* query, lcn_list_t **term_arrays );

/** @} */

/**
 * @defgroup lcn_prefix_query Prefix-Query
 * @ingroup lcn_query
 * @{
 */

/**
 * @brief Creates a new Prefix-Query
 *
 * @param query Result
 * @param pool Memory-Context
 *
 * @result Lucene-Statusvalue
 */

apr_status_t
lcn_prefix_query_create( lcn_query_t** query,
                         const lcn_term_t* term,
                         apr_pool_t* pool );

void
lcn_prefix_query_max_terms_set( lcn_query_t* query,
                                unsigned int max );

unsigned int
lcn_prefix_query_max_terms( const lcn_query_t* query );

apr_status_t
lcn_prefix_query_terms( lcn_query_t*,
                        lcn_list_t** terms,
                        lcn_index_reader_t* reader,
                        apr_pool_t* pool );

/**
 * @brief Convenience-Method, that basically behave like above Method,
 *        but creates Prefix-Term internally for you
 *
 * @param query Result
 * @param field Field to search for the prefix in
 * @param prefix Prefix to look for
 * @param pool Memory-Context
 *
 * @result Lucene-Statusvalue
 */

apr_status_t
lcn_prefix_query_create_by_chars( lcn_query_t** query,
                                  const char* field,
                                  const char* prefix,
                                  apr_pool_t* pool );


lcn_term_t*
lcn_prefix_query_prefix( lcn_query_t* query );

/** @} */

/** @} */

/** @} */

END_C_DECLS


#endif /* LCN_SEARCH_H */
