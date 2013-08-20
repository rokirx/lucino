#include <stdlib.h>
#include <stdio.h>
#include "lucene.h"
#include "test_all.h"
#include <apr_general.h>
#include "fs_field.h"

/* extern FILE* pDebugOut; */

void
setup_test_dir (void)
{
}

void
teardown_test_dir (void)
{
}

int main (void)
{
    CuString *output = CuStringNew();
    CuSuite* sr = CuSuiteNew();
    apr_status_t s;
    apr_pool_t *main_pool;

    lcn_log_stream = stderr;

    do
    {
        if ( ! APR_SUCCESS == ( s = apr_initialize() ) )
        {
            char buf[1000];
            char *err = apr_strerror( s, buf, 1000 );
            fprintf(stderr, "%s", err );
            return 1;
        }

        LCNCE( apr_pool_create( &main_pool, NULL ) );
        LCNCE( lcn_atom_init ( main_pool ) );
    }
    while(0);

    setbuf(stdout, 0);

    CuSuiteAddSuite(sr, make_50_index_writer_suite());
    CuSuiteAddSuite(sr, make_50_checksum_index_output_suite());
    CuSuiteAddSuite(sr, make_ostream_suite());
    CuSuiteAddSuite(sr, make_ram_file_suite() ); 
   
#if 0
    CuSuiteAddSuite(sr, make_50_index_writer_suite());
    CuSuiteAddSuite(sr, make_50_crc32_suite());
    CuSuiteAddSuite(sr, make_ostream_suite());
    CuSuiteAddSuite(sr, make_50_checksum_index_output_suite());
#endif
#if 0
    CuSuiteAddSuite(sr, make_50_document_writer_suite());
    CuSuiteAddSuite(sr, make_50_index_writer_suite());
    CuSuiteAddSuite(sr, make_50_crc32_suite());
    CuSuiteAddSuite(sr, make_50_checksum_index_output_suite());
    CuSuiteAddSuite(sr, make_array_suite() );
    CuSuiteAddSuite(sr, make_atom_suite());
    CuSuiteAddSuite(sr, make_bitvector_suite());
    CuSuiteAddSuite(sr, make_boolean_clause_suite() );
    CuSuiteAddSuite(sr, make_boolean_query_suite() );
    CuSuiteAddSuite(sr, make_char_tokenizer_suite() );
    CuSuiteAddSuite(sr, make_compatibility_suite());
    CuSuiteAddSuite(sr, make_compound_file_suite());
    CuSuiteAddSuite(sr, make_compound_file_util_suite());
    CuSuiteAddSuite(sr, make_custom_hit_queue_suite() );
    CuSuiteAddSuite(sr, make_directory_suite());
    CuSuiteAddSuite(sr, make_document_suite() );
    CuSuiteAddSuite(sr, make_explanation_suite() );
    CuSuiteAddSuite(sr, make_field_fixed_size_suite());
    CuSuiteAddSuite(sr, make_field_infos_suite());
    CuSuiteAddSuite(sr, make_field_sorted_hit_queue_suite() );
    CuSuiteAddSuite(sr, make_filtered_query_suite());
    CuSuiteAddSuite(sr, make_fs_field_bitvector_suite() );
    CuSuiteAddSuite(sr, make_fs_field_query_suite() );
    CuSuiteAddSuite(sr, make_german_stem_filter_suite());
    CuSuiteAddSuite(sr, make_group_search_suite());
    CuSuiteAddSuite(sr, make_index_reader_suite());
    CuSuiteAddSuite(sr, make_index_searcher_suite() );
    CuSuiteAddSuite(sr, make_index_writer_bugs_suite());
    CuSuiteAddSuite(sr, make_index_writer_suite());
    CuSuiteAddSuite(sr, make_input_stream_suite());
    CuSuiteAddSuite(sr, make_linked_list_suite() );
    CuSuiteAddSuite(sr, make_lucene_list_suite());
    CuSuiteAddSuite(sr, make_lucene_util_suite());
    CuSuiteAddSuite(sr, make_match_all_docs_query_suite() );
    CuSuiteAddSuite(sr, make_multi_phrase_query_suite() );
    CuSuiteAddSuite(sr, make_multi_reader_suite());
    CuSuiteAddSuite(sr, make_multiple_fields_suite());
    CuSuiteAddSuite(sr, make_one_hit_query_suite() );
    CuSuiteAddSuite(sr, make_ordered_query_suite() );
    CuSuiteAddSuite(sr, make_ostream_suite());
    CuSuiteAddSuite(sr, make_prefix_query_suite() );
    CuSuiteAddSuite(sr, make_priority_queue_suite() );
    CuSuiteAddSuite(sr, make_query_bitvector_suite() );
    CuSuiteAddSuite(sr, make_query_parser_suite() );
    CuSuiteAddSuite(sr, make_query_tokenizer_suite() );
    CuSuiteAddSuite(sr, make_ram_file_suite() );
    CuSuiteAddSuite(sr, make_range_counting_suite());
    CuSuiteAddSuite(sr, make_score_doc_comparator_suite() );
    CuSuiteAddSuite(sr, make_scorer_suite() );
    CuSuiteAddSuite(sr, make_segment_infos_suite());
    CuSuiteAddSuite(sr, make_segment_term_enum_suite() );
    CuSuiteAddSuite(sr, make_similarity_suite() );
    CuSuiteAddSuite(sr, make_simple_analyzer_suite() );
    CuSuiteAddSuite(sr, make_sort_by_bitvector_suite() );
    CuSuiteAddSuite(sr, make_sort_field_suite() );
    CuSuiteAddSuite(sr, make_stop_filter_suite() );
    CuSuiteAddSuite(sr, make_string_buffer_suite() );
    CuSuiteAddSuite(sr, make_string_suite() );
    CuSuiteAddSuite(sr, make_term_docs_suite());
    CuSuiteAddSuite(sr, make_term_infos_reader_suite());
    CuSuiteAddSuite(sr, make_term_infos_writer_suite());
    CuSuiteAddSuite(sr, make_term_pos_query_suite() );
    CuSuiteAddSuite(sr, make_term_query_suite());
    CuSuiteAddSuite(sr, make_term_suite());
    CuSuiteAddSuite(sr, make_top_doc_collector_suite() );
#endif

    CuSuiteRun(sr);
    CuSuiteSummary(sr, output);
    CuSuiteDetails(sr, output);
    printf("%s\n", output->buffer);
    free( output->buffer );
    free( output );

    CuSuiteFree( sr );

    apr_terminate();

    return 0;
}
