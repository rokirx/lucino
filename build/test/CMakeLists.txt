#CMake Warnung "Libraries muessen gesucht werden" verhindern
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_EXECUTABLE(
    test_lcn
    ../../test/CuTest.c
    ../../test/t_array.c
    ../../test/t_atom.c
    ../../test/t_bitvector.c
    ../../test/t_boolean_clause.c
    ../../test/t_boolean_query.c
    ../../test/t_char_tokenizer.c
    ../../test/t_compatibility.c
    ../../test/t_compound_file.c
    ../../test/t_compound_file_util.c
    ../../test/t_custom_hit_queue.c
    ../../test/t_directory.c
    ../../test/t_document.c
    ../../test/t_explanation.c
    ../../test/t_field_fixed_size.c
    ../../test/t_field_infos.c
    ../../test/t_field_sorted_hit_queue.c
    ../../test/t_filtered_query.c
    ../../test/t_fs_field_bitvector.c
    ../../test/t_fs_field_query.c
    ../../test/t_german_stem_filter.c
    ../../test/t_group_search.c
    ../../test/t_index_reader.c
    ../../test/t_index_searcher.c
    ../../test/t_index_writer.c
    ../../test/t_50_index_writer.c
    ../../test/t_index_writer_bugs.c
    ../../test/t_input_stream.c
    ../../test/t_linked_list.c
    ../../test/t_list.c
    ../../test/t_lucene_util.c
    ../../test/t_match_all_docs_query.c
    ../../test/t_multi_phrase_query.c
    ../../test/t_multi_reader.c
    ../../test/t_multiple_fields.c
    ../../test/t_one_hit_query.c
    ../../test/t_ordered_query.c
    ../../test/t_output_stream.c
    ../../test/t_prefix_query.c
    ../../test/t_priority_queue.c
    ../../test/t_query_bitvector.c
    ../../test/t_query_parser.c
    ../../test/t_query_tokenizer.c
    ../../test/t_ram_file.c
    ../../test/t_range_counting.c
    ../../test/t_score_doc_comparator.c
    ../../test/t_scorers.c
    ../../test/t_segment_infos.c
    ../../test/t_similarity.c
    ../../test/t_simple_analyzer.c
    ../../test/t_sort_by_bitvector.c
    ../../test/t_sort_field.c
    ../../test/t_stop_filter.c
    ../../test/t_string.c
    ../../test/t_string_buffer.c
    ../../test/t_term.c
    ../../test/t_term_docs.c
    ../../test/t_term_enum.c
    ../../test/t_term_infos_reader.c
    ../../test/t_term_infos_writer.c
    ../../test/t_term_pos_query.c
    ../../test/t_term_query.c
    ../../test/t_term_scorer.c
    ../../test/t_top_doc_collector.c
    ../../test/test_all.c
    ../../test/test_util.c
 )

INCLUDE_DIRECTORIES( ${APR_INCLUDE}
                     ../include
                     ../src
                     ../src/analysis
                     ../src/document
                     ../src/index
                     ../src/search
                     ../src/store
                     ../src/util
                     ../src/query_parser )

IF( NV_LOGGING )
  ADD_DEFINITIONS( -D_LCNTRACE )
ENDIF( NV_LOGGING )

IF( NOT WIN32 )

  TARGET_LINK_LIBRARIES( test_lcn
                         lucino
                         ${APR_LIB}
                         m
                         z
                         ${PTHREAD_LIB} )

ELSE( NOT WIN32 )
    TARGET_LINK_LIBRARIES( test_lcn
                           lucino
                           Ws2_32.lib
                           ${APR_LIB} )
ENDIF( NOT WIN32 )

IF( NOT NV_CROSS_COMPILE )
  ADD_CUSTOM_COMMAND( TARGET test_lcn
                      POST_BUILD
                      COMMAND ./test_lcn )
ENDIF( NOT NV_CROSS_COMPILE )
