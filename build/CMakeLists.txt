#CMake Warnung "Libraries muessen gesucht werden" verhindern
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_LIBRARY( lucino
             ../src/analysis/analyzer
             ../src/analysis/analyzer_map
             ../src/analysis/char_tokenizer
             ../src/analysis/de/german_analyzer
             ../src/analysis/de/german_stem_filter
             ../src/analysis/de/german_stemmer
             ../src/analysis/letter_tokenizer
             ../src/analysis/lowercase_filter
             ../src/analysis/lowercase_tokenizer
             ../src/analysis/simple_analyzer
             ../src/analysis/stemmer
             ../src/analysis/stop_filter
             ../src/analysis/token
             ../src/analysis/whitespace_analyzer
             ../src/analysis/whitespace_tokenizer
             ../src/document/document
             ../src/document/document_dump_reader
             ../src/document/field
             ../src/document/field_type
             ../src/index/directory_reader
             ../src/index/document_writer
             ../src/index/field_infos
             ../src/index/fields_reader
             ../src/index/fields_writer
             ../src/index/fs_field
             ../src/index/index_reader
             ../src/index/index_writer
             ../src/index/multi_fs_field
             ../src/index/multi_reader
             ../src/index/multiple_term_positions
             ../src/index/segment_info
             ../src/index/segment_info_per_commit
             ../src/index/segment_infos
             ../src/index/segment_merge_info
             ../src/index/segment_merger
             ../src/index/segment_reader
             ../src/index/term
             ../src/index/term_docs
             ../src/index/term_enum
             ../src/index/term_infos_reader
             ../src/index/term_infos_writer
             ../src/index/index_writer_config
             ../src/query_parser/query_parser
             ../src/query_parser/query_tokenizer
             ../src/search/boolean_clause
             ../src/search/boolean_query
             ../src/search/boolean_scorer
             ../src/search/conjunction_scorer
             ../src/search/coordinator
             ../src/search/counting_conjunction_sum_scorer
             ../src/search/counting_disjunction_sum_scorer
             ../src/search/disjunction_sum_scorer
             ../src/search/explanation
             ../src/search/field_sorted_hit_queue
             ../src/search/filtered_query
             ../src/search/hit_collector
             ../src/search/hit_queue
             ../src/search/hits
             ../src/search/index_searcher
             ../src/search/match_all_docs_query
             ../src/search/multi_phrase_query
             ../src/search/non_matching_scorer
             ../src/search/phrase_positions
             ../src/search/phrase_queue
             ../src/search/phrase_scorer
             ../src/search/prefix_query
             ../src/search/query
             ../src/search/req_excl_scorer
             ../src/search/req_opt_sum_scorer
             ../src/search/score_doc_comparator
             ../src/search/scorer
             ../src/search/scorer_queue
             ../src/search/searcher
             ../src/search/similarity
             ../src/search/single_match_scorer
             ../src/search/sort_field
             ../src/search/term_pos_query
             ../src/search/term_pos_scorer
             ../src/search/term_query
             ../src/search/term_scorer
             ../src/search/top_doc_collector
             ../src/search/top_docs
             ../src/search/weight
             ../src/store/directory
             ../src/store/index_input
             ../src/store/ostream
             ../src/store/compound_file_reader
             ../src/store/compound_file_writer
             ../src/store/compound_file_util
             ../src/store/ram_directory
             ../src/store/ram_input_stream
             ../src/util/array
             ../src/util/atom
             ../src/util/bitvector
             ../src/util/errorcodes
             ../src/util/linked_list
             ../src/util/list
             ../src/util/priority_queue
             ../src/util/smallfloat
             ../src/util/string_buffer
             ../src/util/to_string_utils
             ../src/util/util )

INCLUDE_DIRECTORIES( ${APR_INCLUDE}
                     ../include
                     ../src
                     ../src/analysis
                     ../src/document
                     ../src/index
                     ../src/search
                     ../src/store
                     ../src/util
                     ../examples/
                     ../src/query_parser )

#ADD_DEFINITIONS( -D_LCNTRACE )

IF( NOT WIN32 )
    ADD_EXECUTABLE(
        indexinfo
        ../examples/indexinfo_func.c
        ../examples/indexinfo.c )

    TARGET_LINK_LIBRARIES( lucino ${APR_LIB} m z )

    TARGET_LINK_LIBRARIES( indexinfo
                           ${APR_LIB} lucino m z ${PTHREAD_LIB} )

ELSE( NOT WIN32 )
    TARGET_LINK_LIBRARIES( lucino ${APR_LIB} )
ENDIF( NOT WIN32 )

add_subdirectory( test )
