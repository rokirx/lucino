#ifndef SEGMENT_INFO_H
#define SEGMENT_INFO_H

#include "lucene.h"
#include "term.h"

typedef struct lcn_segment_info_t lcn_segment_info_t;


#define LCN_SEGMENT_INFOS_FORMAT (-1)

struct lcn_segment_info_t {

    lcn_directory_t *directory;
    char *name;
    unsigned int doc_count;
};

struct lcn_segment_infos_t {

    int format;
    unsigned int counter;
    lcn_list_t *list;

    apr_pool_t *subpool;

    /**
     *  Lucene 4.0
     */

    apr_pool_t *pool;

    /**
     * List<SegmentInfoPerCommit>
     */
    lcn_list_t *segments;

    /**
     * generation of the "segments_N" for the next commit
     */
    apr_int64_t generation;

    /**
     * Counts how often the index has been changed.
     */
    apr_uint64_t version;

    /**
     * generation of the "segments_N" file we last successfully read
     * or wrote; this is normally the same as generation except if
     * there was an IOException that had interrupted a commit
     */
    apr_int64_t last_generation;

};

const char *
lcn_segment_info_name( lcn_segment_info_t *segment_info );

lcn_directory_t *
lcn_segment_info_directory( lcn_segment_info_t *segment_info );

unsigned int
lcn_segment_info_doc_count( lcn_segment_info_t *segment_info );

unsigned int
lcn_segment_infos_size( lcn_segment_infos_t *segment_infos );

apr_status_t
lcn_segment_infos_get( lcn_segment_infos_t *segment_infos,
                       lcn_segment_info_t **segment_info,
                       unsigned int nth );

apr_status_t
lcn_segment_info_has_deletions( lcn_segment_info_t *segment_info,
                                lcn_bool_t *has_deletions );

void
lcn_segment_infos_remove( lcn_segment_infos_t *segment_infos, unsigned int i );

apr_status_t
lcn_segment_infos_get_next_name( lcn_segment_infos_t *segment_infos,
                                 char **seg_name,
                                 apr_pool_t *pool );

apr_status_t
lcn_segment_infos_add_info( lcn_segment_infos_t *segment_infos,
                            lcn_directory_t *directory,
                            const char *name,
                            unsigned int count );

apr_status_t
lcn_segment_infos_write( lcn_segment_infos_t *segment_infos,
                         lcn_directory_t *directory );

apr_status_t
lcn_segment_infos_create( lcn_segment_infos_t **segment_infos,
                          apr_pool_t *pool );

apr_uint64_t
lcn_segment_infos_version( lcn_segment_infos_t *segment_infos );

int
lcn_segment_infos_format( lcn_segment_infos_t *segment_infos );

apr_status_t
lcn_segment_infos_has_separate_norms( lcn_segment_info_t *segment_info,
                                      lcn_bool_t *flag,
                                      apr_pool_t *pool);

/**
 * Lucene 4.0
 */

void
lcn_segment_infos_changed( lcn_segment_infos_t *segment_infos );


/**
 * Clear all {@link SegmentInfoPerCommit}s.
 */
void
lcn_segment_infos_clear( lcn_segment_infos_t *segment_infos );


/**
 * Find the latest commit ({@code segments_N file}) and
 * load all {@link SegmentInfoPerCommit}s.
 */
apr_status_t
lcn_segment_infos_read_directory( lcn_segment_infos_t *segment_infos,
                                  lcn_directory_t *directory );


#endif
