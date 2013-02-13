#ifndef READERS_AND_LIVE_DOCS_H
#define READERS_AND_LIVE_DOCS_H

#include "lucene.h"
#include "index_writer.h"
#include "segment_infos.h"

/**
 * Type definitions
 */

typedef struct lcn_readers_and_live_docs_t
{
    lcn_segment_info_per_commit_t *segment_info;

    lcn_segment_info_per_commit_t *info;

    lcn_index_writer_t *writer;

    /**
     * True if the current lieveDocs is referenced by an
     * external NRT reader
     */
    lcn_bool_t shared;

    /**
     * Tracks how many consumers are using this Instance:
     */
    unsigned int ref_count;

    /**
     * How many future deletions we've done against
     * LiveDocs vs when we loaded it or last wrote it:
     */
    unsigned int pending_delete_count;

} lcn_readers_and_live_docs_t;

/**
 * Functions
 */

apr_status_t
lcn_readers_and_live_docs_create( lcn_readers_and_live_docs_t **rld,
                                  lcn_index_writer_t *index_writer,
                                  lcn_segment_info_per_commit_t *info,
                                  apr_pool_t *pool );
void
lcn_readers_and_live_docs_increment( lcn_readers_and_live_docs_t *rld );

#endif
