#ifndef INDEX_WRITER_H
#define INDEX_WRITER_H

#include "lucene.h"

#include "segment_infos.h"
#include "term_docs.h"

/**
 * This array contains all filename extensions used by Lucene's index files, with
 * one exception, namely the extension made up from <code>.f</code> + a number.
 * Also note that two of Lucene's files (<code>deletable</code> and
 * <code>segments</code>) don't have any filename extension.
 */
extern char* lcn_index_extensions[];

#define LCN_INDEX_EXTENSIONS_SIZE (13)


#define LCN_INDEX_WRITER_DEFAULT_MAX_FIELD_LENGTH    (10000)
#define LCN_INDEX_WRITER_DEFAULT_TERM_INDEX_INTERVAL   (128)
#define LCN_INDEX_WRITER_DEFAULT_MAX_BUFFERED_DOCS      (10)
#define LCN_INDEX_WRITER_DEFAULT_MAX_MERGE_DOCS      (JAVA_INTEGER_MAX_VALUE)
#define LCN_INDEX_WRITER_DEFAULT_MERGE_FACTOR           (10)

struct lcn_index_writer_t {

    apr_pool_t *pool;
    apr_pool_t *ram_dir_subpool;
    apr_pool_t *seg_name_subpool;

    unsigned int max_field_length;
    unsigned int term_index_interval;
    unsigned int max_buffered_docs;
    unsigned int max_merge_docs;
    unsigned int merge_factor;

    int seg_name_counter;

    lcn_bool_t close_dir;
    lcn_directory_t *directory;
    lcn_segment_infos_t *segment_infos;
    lcn_directory_t *ram_directory;
    lcn_similarity_t *similarity;

    FILE *log_stream;

    apr_hash_t *fs_fields;
    unsigned int docs_count;
};




#endif /* INDEX_WRITER_H */
