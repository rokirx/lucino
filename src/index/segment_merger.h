#ifndef SEGMENT_MERGER_H
#define SEGMENT_MERGER_H

#include "index_reader.h"
#include "directory.h"
#include "index_writer.h"
#include "lcn_util.h"
#include "field_infos.h"
#include "term_infos_writer.h"

typedef struct lcn_segment_merger_t lcn_segment_merger_t;

struct lcn_segment_merger_t {

    apr_pool_t *pool;

    lcn_list_t *readers;
    lcn_directory_t *directory;
    char *segment;
    unsigned int term_index_interval;
    unsigned int skip_interval;
    lcn_field_infos_t *field_infos;
    lcn_term_info_t *term_info;

    /* this elements are not initialized by default */
    lcn_index_output_t *freq_output;
    lcn_index_output_t *prox_output;
    lcn_term_infos_writer_t *ti_writer;
    lcn_priority_queue_t *queue;

    lcn_index_output_t *skip_buffer;
    unsigned int last_skip_doc;
    apr_off_t last_skip_freq_pointer;
    apr_off_t last_skip_prox_pointer;


#if 0
    char old_segment[MAX_LEN_SEGMENT_NAME];
    bool use_compound_file;

    /**
     * Merges the readers specified by the {@link #add} method into the
     * directory passed to the constructor
     *
     * @return error code
     */
    int (*merge) ( struct segment_merger *, int* merged_doc_count );

#endif
};


/**
 * @param dir The lcn_directory_t to merge the other segments into
 * @param name The name of the new segment
 * @param use_compound_file true if the new segment should use a compoundFile
 */
apr_status_t
lcn_segment_merger_create ( lcn_segment_merger_t **segment_merger,
                            lcn_index_writer_t *index_writer,
                            const char *name,
                            apr_pool_t *pool );

/**
 * Add an IndexReader to the collection of readers that are to be merged
 * @param reader
 */
apr_status_t
lcn_segment_merger_add_reader ( lcn_segment_merger_t *segment_merger,
                                lcn_index_reader_t *index_reader );

/**
 * Merges the readers specified by the add method into the directory
 * passed to the constructor
 *
 * unsigned int *merged_doc_count  stores the number of documents that were merged
 */
apr_status_t
lcn_segment_merger_merge( lcn_segment_merger_t *segment_merger,
                          unsigned int *merged_doc_count );

/*
 * close all IndexReaders that have been added.
 * Should not be called before merge().
 * @throws IOException
 */
apr_status_t
lcn_segment_merger_close_readers( lcn_segment_merger_t *segment_merger );

/**
 * Select all files for cfs-output and merge them together.
 */
apr_status_t
lcn_segment_merger_create_compound_file( lcn_segment_merger_t *sm,
                                         char *cf_name,
                                         lcn_directory_t *dir,
                                         lcn_list_t **files,
                                         apr_pool_t *pool);



#endif /* SEGMENT_MERGER_H */
