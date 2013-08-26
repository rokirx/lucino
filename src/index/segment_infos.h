#ifndef SEGMENT_INFO_H
#define SEGMENT_INFO_H

#include "lucene.h"
#include "term.h"
#include "lcn_index.h"
#include "index_file_names.h"

typedef struct lcn_segment_info_t lcn_segment_info_t;
//typedef struct lcn_segment_info_per_commit_t lcn_segment_info_per_commit_t;
//typedef struct lcn_segment_infos_t lcn_segment_infos_t;

#define LCN_SEGMENT_INFOS_FORMAT (-1)

/**
 * The file format version for the segments_N codec header
 */
#define LCN_SEGMENT_INFOS_VERSION_40 ( 0 )

struct lcn_segment_info_per_commit_t
{
    lcn_segment_info_t *segment_info;

    /**
     * How many deleted docs in the segment
     */
    unsigned int del_count;

    /**
     * Lucene 5.0
     */

    /**
     * TDOD: implement. del_gen is never set
     *
     * Returns the number of deleted docs in the segment.
     */
    unsigned int del_gen;
};

struct lcn_segment_info_t {

    lcn_directory_t *directory;
    char *name;
    unsigned int doc_count;

    /**
     * Lucene 4.0
     */

    char* version;

    unsigned int is_compound_file;

    /**
     * Lucene 5.0
     */

    lcn_list_t *set_files;
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

    /**
     * Lucene 5.0
     */

    /**
     * must create with lcn_checksum_index_output_create()
     */
    lcn_index_output_t *pending_seqn_output;

   /*
    * Opaque Map&lt;String, String&gt; that user can specify during IndexWriter.commit
    */
    apr_hash_t *user_data;
};

lcn_segment_info_t *
lcn_segment_info_per_commit_info( lcn_segment_info_per_commit_t *info_pc );


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
                       lcn_segment_info_per_commit_t **segment_info,
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

apr_status_t
lcn_segment_info_to_string( char** str,
                            lcn_segment_info_t *segment_info,
                            lcn_directory_t *dir,
                            unsigned int del_count,
                            apr_pool_t *pool);

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

apr_status_t
lcn_segment_info_per_commit_to_string( char** str,
                                       lcn_segment_info_per_commit_t *info_pc,
                                       lcn_directory_t *directory,
                                       unsigned int pending_del_count,
                                       apr_pool_t *pool );
char*
lcn_segment_info_per_commit_to_hash( lcn_segment_info_per_commit_t *info_pc,
                                     apr_pool_t *pool);

apr_status_t
lcn_segment_info_per_commit_num_deleted_docs( lcn_segment_info_per_commit_t *segment_info,
                                              unsigned int *del_count,
                                              lcn_index_writer_t *index_writer,
                                              apr_pool_t *pool );

/**
 * Lucene 5.0
 */


apr_status_t
lcn_segment_infos_files( lcn_segment_infos_t *segement_infos,
                         lcn_directory_t *dir,
                         lcn_bool_t include_segments_file,
                         apr_pool_t *pool,
                         lcn_list_t **files );

apr_status_t
lcn_segment_infos_clone( lcn_segment_infos_t **clone,
                         lcn_segment_infos_t *segment_infos,
                         apr_pool_t *pool );

void
lcn_segment_infos_update_generation( lcn_segment_infos_t *clone,
                                     lcn_segment_infos_t *segment_infos );

apr_status_t
lcn_segment_infos_finish_commit( lcn_segment_infos_t *pending_commit,
                                 lcn_directory_t *directory );

apr_status_t
lcn_segment_info_files( lcn_segment_info_t *segment_info,
                        apr_pool_t *pool,
                        apr_hash_t **files );

/** Call this to start a commit.  This writes the new
 *  segments file, but writes an invalid checksum at the
 *  end, so that it is not visible to readers.  Once this
 *  is called you must call {@link #finishCommit} to complete
 *  the commit or {@link #rollbackCommit} to abort it.
 *  <p>
 *  Note: {@link #changed()} should be called prior to this
 *  method if changes have been made to this {@link SegmentInfos} instance
 *  </p>
 **/
apr_status_t
lcn_segment_infos_prepare_commit( lcn_segment_infos_t *segment_infos,
                                  lcn_directory_t *dir );


lcn_segment_info_per_commit_t*
lcn_segment_info_per_commit_clone( lcn_segment_info_per_commit_t *segment_info_per_commit,
                                    apr_pool_t *pool );

/**
 * Call this before committing if changes have been made to the
 * segments.
 */
void
lcn_segment_infos_changed( lcn_segment_infos_t *segment_infos );

/**
 * Parse the generation off the segments file name and
 * return it.
 */
apr_status_t
lcn_segment_infos_generation_from_segments_file_name( char* filename, apr_int64_t* generation );

#endif

