#ifndef TERM_ENUM_H
#define TERM_ENUM_H

#include "lucene.h"
#include "lcn_index.h"

#include "field_infos.h"
#include "lcn_store.h"


struct lcn_term_enum_t {

    apr_pool_t *pool;
    apr_pool_t *buffer_pool;
    apr_pool_t *save_pool;


    /* SegmentTermEnum */

    lcn_istream_t     *istream;
    lcn_field_infos_t *field_infos;

    unsigned int index_interval;
    unsigned int skip_interval;

    /**
     * Current term of the enumeration. May be NULL if the last call
     * of next returned FALSE.
     */
    lcn_term_t *term;
    lcn_term_t *prev;

    char *term_text;
    char *prev_text;
    lcn_term_info_t *term_info;

    unsigned int buffer_size;

    /**
     * is TRUE if the instance is a clone
     */
    lcn_bool_t is_clone;

    /**
     * is TRUE if the enumeration of an Index (file extenstion tii)
     */
    lcn_bool_t is_index;
    apr_off_t index_pointer;


    unsigned int (*get_doc_freq) (lcn_term_enum_t *term_enum);
    apr_status_t (*next) ( lcn_term_enum_t *term_enum );
    apr_status_t (*close) (lcn_term_enum_t *term_enum );

    lcn_bool_t skip_first_next;

    /* TermEnum */

    apr_uint64_t size;
    apr_off_t position;

    /* MultiTermEnum */
    lcn_priority_queue_t *queue;
    int doc_freq;
    lcn_index_reader_t **sub_readers;
    unsigned int sub_readers_count;
};

apr_status_t
lcn_segment_term_enum_create( lcn_term_enum_t **term_enum,
                              lcn_istream_t *istream,
                              lcn_field_infos_t *field_infos,
                              lcn_bool_t is_index,
                              apr_pool_t *pool );

apr_status_t
lcn_multi_term_enum_create( lcn_term_enum_t **term_enum,
                            lcn_index_reader_t **sub_readers,
                            unsigned int *starts,
                            unsigned int sub_readers_count,
                            lcn_term_t *term,
                            apr_pool_t *pool );


#endif
