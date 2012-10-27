#ifndef SEGMENT_MERGE_INFO_H
#define SEGMENT_MERGE_INFO_H

#include "lucene.h"
#include "lcn_util.h"
#include "lcn_index.h"

struct lcn_segment_merge_info_t
{
    apr_pool_t *pool;

    int base;
    const lcn_term_t *term;
    lcn_index_reader_t *reader;
    lcn_term_enum_t *term_enum;
    lcn_term_docs_t *postings;
    lcn_int_array_t *doc_map;
};

#endif /* SEGMENT_MERGE_INFO_H */
