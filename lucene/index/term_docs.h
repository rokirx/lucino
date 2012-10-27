#ifndef TERM_DOCS_H
#define TERM_DOCS_H

#include "lucene.h"
#include "lcn_store.h"
#include "lcn_term_enum.h"
#include "lcn_index.h"

#include "term_infos_reader.h"


/* {{{ struct lcn_term_docs_t */

typedef struct lcn_term_docs_private_t lcn_term_docs_private_t;

struct lcn_term_docs_t {

    apr_pool_t *pool;

    lcn_term_docs_private_t* priv;

    lcn_index_reader_t *parent;
    lcn_istream_t *freq_stream;
    lcn_istream_t *skip_stream;
    lcn_bitvector_t *deleted_docs;
    unsigned int skip_interval;
    unsigned int count;

    /**
     * document frequency
     */
    unsigned int df;

    unsigned int doc;
    unsigned int freq;

    /**
     *
     */
    unsigned int skip_doc;
    unsigned int skip_count;
    unsigned int num_skips;

    apr_off_t freq_pointer;
    apr_off_t prox_pointer;
    apr_off_t skip_pointer;

    lcn_bool_t have_skipped;

    apr_status_t (*seek_term) ( lcn_term_docs_t *term_docs,
                                const lcn_term_t *term );

    apr_status_t (*seek_term_info) ( lcn_term_docs_t *term_docs,
                                     lcn_term_info_t *term_info );

    apr_status_t (*seek_term_enum) ( lcn_term_docs_t *term_docs,
                                     lcn_term_enum_t *term_enum );

    apr_status_t (*close) ( lcn_term_docs_t *term_docs );
    apr_status_t (*next) (lcn_term_docs_t *term_docs );
    apr_status_t (*skipping_doc) (lcn_term_docs_t *term_docs );
    apr_status_t (*skip_prox) (lcn_term_docs_t *term_docs, apr_off_t prox_pointer );
    apr_status_t (*skip_to) ( lcn_term_docs_t *term_docs, unsigned int target );
    apr_status_t (*read) ( lcn_term_docs_t *term_docs,
                           lcn_int_array_t* docs,
                           lcn_int_array_t* freqs,
                           unsigned int *read_entries );


    unsigned int (*get_doc) ( lcn_term_docs_t *term_docs );
    unsigned int (*get_freq) ( lcn_term_docs_t *term_docs );

    /* TermPositions */
    lcn_istream_t *prox_stream;
    int prox_count;
    unsigned int position;

    apr_status_t (*next_position) ( lcn_term_docs_t *term_positions,
                                    apr_ssize_t *position );

    /* MultiTermDocs */
    lcn_term_docs_t **reader_term_docs;
    lcn_index_reader_t **readers;
    unsigned int *starts;
    unsigned int readers_count;
    lcn_term_docs_t *current;
    int pointer;
    int base;
    const lcn_term_t *term;

    apr_status_t (*term_docs) ( lcn_term_docs_t **term_positions,
                                lcn_index_reader_t *index_reader,
                                apr_pool_t *pool );
};

apr_status_t
lcn_multiple_term_positions_create( lcn_term_docs_t** mtp,
                                    lcn_index_reader_t* reader,
                                    lcn_list_t* terms,
                                    apr_pool_t* pool );
/* }}} */

apr_status_t
lcn_segment_term_docs_create( lcn_index_reader_t *index_reader,
                              lcn_term_docs_t **term_docs,
                              apr_pool_t *pool );

apr_status_t
lcn_segment_term_positions_create( lcn_index_reader_t *index_reader,
                                   lcn_term_docs_t **term_docs,
                                   apr_pool_t *pool );

apr_status_t
lcn_multi_term_docs_create( lcn_term_docs_t **term_docs,
                            lcn_index_reader_t **readers,
                            unsigned int *starts,
                            int readers_count,
                            apr_pool_t *pool );

apr_status_t
lcn_multi_term_positions_create( lcn_term_docs_t **term_docs,
                                 lcn_index_reader_t **readers,
                                 unsigned int *starts,
                                 int readers_count,
                                 apr_pool_t *pool );


#endif /* TERM_DOCS_H */
