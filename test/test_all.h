#ifndef TEST_ALL_H
#define TEST_ALL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "apr_pools.h"
#include "lucene.h"
#include "directory.h"
#include "CuTest.h"

#define TEST_DIR "./index/"
#define TEST_DIR_LEN strlen(TEST_DIR)

#define WRITE_DIR "./temp_index/"
#define WRITE_DIR_LEN strlen(WRITE_DIR)

extern char *data[];

#define LCN_TEST_STATUS( STATEMENT, STATUS_EXPECTED )                   \
{                                                                       \
        char error_buf[1000];                                           \
        char exp_buf[1000];                                             \
                                                                        \
        lcn_strerror( STATUS_EXPECTED, exp_buf, 1000 );                 \
        lcn_strerror( ( STATEMENT ), error_buf, 1000 );                 \
        CuAssertStrEquals( tc, exp_buf, error_buf );                    \
}

#define LCN_TEST( STATEMENT ) LCN_TEST_STATUS( STATEMENT, APR_SUCCESS )

#define LCN_ERR(statement, ERR) CuAssertIntEquals(tc, ERR, statement )

#define ASSERT_APR_SUCCESS( FUNC )                                      \
{                                                                       \
    char error_buf[1000];                                               \
                                                                        \
    if( ( s = FUNC ) )                                                  \
    {                                                                   \
        fprintf( stderr, "Error in <%s>, line %d: %s\n", __FILE__, __LINE__, lcn_strerror( s, error_buf, 1000 ) ); \
    }                                                                   \
                                                                        \
                                                                        \
    CuAssertIntEquals( tc, APR_SUCCESS, s );                            \
}

#define CHECK_HIT( HITS, NTH, FIELD, TEXT )                     \
{                                                               \
    lcn_document_t* doc;                                        \
    char* content;                                              \
                                                                \
    LCN_TEST( lcn_hits_doc( HITS, &doc, NTH, pool ) );          \
    LCN_TEST( lcn_document_get( doc, &content, FIELD, pool ) ); \
    CuAssertStrEquals( tc, TEXT, content );                     \
}

apr_pool_t *main_pool;

void
compare_input_streams( CuTest* tc,
                       lcn_index_input_t *is_a,
                       lcn_index_input_t *is_b );

void
compare_directories(CuTest* tc,
                    const char* path_a,
                    const char* path_b );

void
delete_files( CuTest* tc,
              const char* path );

void
create_index( CuTest *tc,
              unsigned int start,
              unsigned int end,
              const char *path,
              lcn_bool_t optimize,
              apr_pool_t *pool );

void
create_index_cf( CuTest *tc,
              unsigned int start,
              unsigned int end,
              const char *path,
              lcn_bool_t optimize_cf,
              apr_pool_t *pool );

void
create_index_by_dump( CuTest *tc,
                      const char* dump_file,
                      const char* index_dir,
                      apr_pool_t *pool );

void setup_test_dir(void);
void teardown_test_dir(void);

CuSuite* make_50_document_writer_suite( void );
CuSuite* make_50_index_writer_suite( void );
CuSuite* make_array_suite( void );
CuSuite* make_atom_suite(void);
CuSuite* make_bitvector_suite( void );
CuSuite* make_boolean_clause_suite( void );
CuSuite* make_boolean_query_suite(void);
CuSuite* make_char_tokenizer_suite( void );
CuSuite* make_compatibility_suite(void);
CuSuite* make_compound_file_suite(void);
CuSuite* make_compound_file_util_suite(void);
CuSuite* make_custom_hit_queue_suite( void );
CuSuite* make_directory_suite(void);
CuSuite* make_document_suite(void);
CuSuite* make_explanation_suite( void );
CuSuite* make_field_fixed_size_suite( void );
CuSuite* make_field_infos_suite(void);
CuSuite* make_field_sorted_hit_queue_suite(void);
CuSuite* make_filtered_query_suite(void);
CuSuite* make_fs_field_bitvector_suite(void);
CuSuite* make_fs_field_query_suite( void );
CuSuite* make_german_stem_filter_suite( void );
CuSuite* make_group_search_suite( void );
CuSuite* make_index_reader_suite(void);
CuSuite* make_index_searcher_suite( void );
CuSuite* make_index_writer_bugs_suite (void);
CuSuite* make_index_writer_suite(void);
CuSuite* make_input_stream_suite(void);
CuSuite* make_linked_list_suite( void );
CuSuite* make_lucene_list_suite(void);
CuSuite* make_lucene_util_suite(void);
CuSuite* make_match_all_docs_query_suite(void);
CuSuite* make_multi_phrase_query_suite( void );
CuSuite* make_multi_reader_suite (void);
CuSuite* make_multiple_fields_suite (void);
CuSuite* make_one_hit_query_suite(void);
CuSuite* make_ordered_query_suite(void);
CuSuite* make_ostream_suite(void);
CuSuite* make_prefix_query_suite( void );
CuSuite* make_priority_queue_suite( void );
CuSuite* make_query_bitvector_suite( void );
CuSuite* make_query_parser_suite (void);
CuSuite* make_query_tokenizer_suite(void);
CuSuite* make_ram_file_suite (void);
CuSuite* make_range_counting_suite(void);
CuSuite* make_score_doc_comparator_suite( void );
CuSuite* make_scorer_suite( void );
CuSuite* make_segment_infos_suite(void) ;
CuSuite* make_segment_term_enum_suite(void);
CuSuite* make_similarity_suite( void );
CuSuite* make_simple_analyzer_suite( void );
CuSuite* make_sort_by_bitvector_suite(void);
CuSuite* make_sort_field_suite(void);
CuSuite* make_stop_filter_suite( void );
CuSuite* make_string_buffer_suite( void );
CuSuite* make_string_suite( void );
CuSuite* make_term_docs_suite(void);
CuSuite* make_term_infos_reader_suite(void);
CuSuite* make_term_infos_writer_suite(void);
CuSuite* make_term_pos_query_suite(void);
CuSuite* make_term_query_suite(void );
CuSuite* make_term_suite(void);
CuSuite* make_top_doc_collector_suite(void);


#endif
