#ifndef TERM_INFOS_WRITER_H
#define TERM_INFOS_WRITER_H

#define LCN_TERM_INFOS_SKIP_INTERVAL    (16)
#define LCN_TERM_INFOS_FILE_FORMAT      (-2)
#define LCN_TERM_INFO_INITIAL_BUFFER  (1024)

typedef struct _lcn_term_infos_writer_t {

    apr_pool_t *pool;

    /**
     * The output is written to this output stream;
     */
    lcn_ostream_t *output;

    /**
     * Last written term. For efficient memory usage
     * we allocate LCN_TERM_INFO_INITIAL_BUFFER bytes
     * to contain the term text. If the buffer is too
     * small, it is reallocated to double size and so on.
     */
    lcn_term_t last_term;
    unsigned int last_term_buf_size;

    lcn_term_info_t last_ti;
    int last_field_number;

    /**
     * Number of terms already written
     */
    int size;
    int is_open;
    int last_index_pointer;
    int is_index;

    struct _lcn_term_infos_writer_t *other;

    /** Expert: The fraction of terms in the "dictionary" which should be stored
     * in RAM.  Smaller values use more memory, but make searching slightly
     * faster, while larger values use less memory and make searching slightly
     * slower.  Searching is typically not dominated by dictionary lookup, so
     * tweaking this is rarely useful.
     */
    unsigned int index_interval;

    /** Expert: The fraction of {@link TermDocs} entries stored in skip tables,
     * used to accellerate {@link TermDocs#skipTo(int)}.  Larger values result in
     * smaller indexes, greater acceleration, but fewer accelerable cases, while
     * smaller values result in bigger indexes, less acceleration and more
     * accelerable cases. More detailed experiments would be useful here.
     */
    unsigned int skip_interval;

} lcn_term_infos_writer_t;


apr_status_t
lcn_term_infos_writer_close ( lcn_term_infos_writer_t *term_infos_writer );

/**
 * Adds a new <Term, TermInfo> pair to the set.
 * Term must be lexicographically greater than all previous Terms added.
 * TermInfo pointers must be positive and greater than all previous.
 */
apr_status_t
lcn_term_infos_writer_add_term ( lcn_term_infos_writer_t *term_infos_writer,
                                 const lcn_term_t *term,
                                 lcn_term_info_t *term_info,
                                 unsigned int field_number );

apr_status_t
lcn_term_infos_writer_create( lcn_term_infos_writer_t **term_infos_writer,
                              lcn_directory_t *directory,
                              const char *seg_name,
                              unsigned int interval,
                              apr_pool_t *pool );




#endif
