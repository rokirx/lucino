cmake_minimum_required(VERSION 2.4)

#CMake Warnung "Libraries muessen gesucht werden" verhindern
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_LIBRARY( lucene
             ../lucene/analysis/analyzer
             ../lucene/analysis/analyzer_map
             ../lucene/analysis/char_tokenizer
             ../lucene/analysis/de/german_analyzer
             ../lucene/analysis/de/german_stem_filter
             ../lucene/analysis/de/german_stemmer
             ../lucene/analysis/letter_tokenizer
             ../lucene/analysis/lowercase_filter
             ../lucene/analysis/lowercase_tokenizer
             ../lucene/analysis/simple_analyzer
             ../lucene/analysis/stemmer
             ../lucene/analysis/stop_filter
             ../lucene/analysis/token
             ../lucene/analysis/whitespace_analyzer
             ../lucene/analysis/whitespace_tokenizer
             ../lucene/document/document
             ../lucene/document/document_dump_reader
             ../lucene/document/field
             ../lucene/document/field_type
             ../lucene/index/directory_reader
             ../lucene/index/document_writer
             ../lucene/index/field_infos
             ../lucene/index/fields_reader
             ../lucene/index/fields_writer
             ../lucene/index/fs_field
             ../lucene/index/index_reader
             ../lucene/index/index_writer
             ../lucene/index/multi_fs_field
             ../lucene/index/multi_reader
             ../lucene/index/multiple_term_positions
             ../lucene/index/segment_infos
             ../lucene/index/segment_merge_info
             ../lucene/index/segment_merger
             ../lucene/index/segment_reader
             ../lucene/index/term
             ../lucene/index/term_docs
             ../lucene/index/term_enum
             ../lucene/index/term_infos_reader
             ../lucene/index/term_infos_writer
             ../lucene/index/index_writer_config
             ../lucene/query_parser/query_parser
             ../lucene/query_parser/query_tokenizer
             ../lucene/search/boolean_clause
             ../lucene/search/boolean_query
             ../lucene/search/boolean_scorer
             ../lucene/search/conjunction_scorer
             ../lucene/search/coordinator
             ../lucene/search/counting_conjunction_sum_scorer
             ../lucene/search/counting_disjunction_sum_scorer
             ../lucene/search/disjunction_sum_scorer
             ../lucene/search/explanation
             ../lucene/search/field_sorted_hit_queue
             ../lucene/search/filtered_query
             ../lucene/search/hit_collector
             ../lucene/search/hit_queue
             ../lucene/search/hits
             ../lucene/search/index_searcher
             ../lucene/search/match_all_docs_query
             ../lucene/search/multi_phrase_query
             ../lucene/search/non_matching_scorer
             ../lucene/search/phrase_positions
             ../lucene/search/phrase_queue
             ../lucene/search/phrase_scorer
             ../lucene/search/prefix_query
             ../lucene/search/query
             ../lucene/search/req_excl_scorer
             ../lucene/search/req_opt_sum_scorer
             ../lucene/search/score_doc_comparator
             ../lucene/search/scorer
             ../lucene/search/scorer_queue
             ../lucene/search/searcher
             ../lucene/search/similarity
             ../lucene/search/single_match_scorer
             ../lucene/search/sort_field
             ../lucene/search/term_pos_query
             ../lucene/search/term_pos_scorer
             ../lucene/search/term_query
             ../lucene/search/term_scorer
             ../lucene/search/top_doc_collector
             ../lucene/search/top_docs
             ../lucene/search/weight
             ../lucene/store/directory
             ../lucene/store/istream
             ../lucene/store/ostream
             ../lucene/store/compound_file_reader
             ../lucene/store/compound_file_writer
             ../lucene/store/compound_file_util
             ../lucene/util/array
             ../lucene/util/atom
             ../lucene/util/bitvector
             ../lucene/util/errorcodes
             ../lucene/util/linked_list
             ../lucene/util/list
             ../lucene/util/priority_queue
             ../lucene/util/smallfloat
             ../lucene/util/string_buffer
             ../lucene/util/to_string_utils
             ../lucene/util/util )

LINK_DIRECTORIES( ../build/examples )

INCLUDE_DIRECTORIES( ${APR_INCLUDE}
                     ../include
                     ../lucene
                     ../lucene/analysis
                     ../lucene/document
                     ../lucene/index
                     ../lucene/search
                     ../lucene/store
                     ../lucene/util
                     ../examples/
                     ../lucene/query_parser )

ADD_DEFINITIONS( -D_LCNTRACE )

IF( NOT WIN32 )
    ADD_EXECUTABLE(
        indexinfo
        ../examples/indexinfo_func.c
        ../examples/indexinfo.c )

    TARGET_LINK_LIBRARIES( lucene ${APR_LIB} m z )

    TARGET_LINK_LIBRARIES( indexinfo
                           ${APR_LIB} lucene m z ${PTHREAD_LIB} )

ELSE( NOT WIN32 )
    TARGET_LINK_LIBRARIES( lucene ${APR_LIB} )
ENDIF( NOT WIN32 )

SUBDIRS( test examples )
