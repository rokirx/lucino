#ifndef DOCUMENT_WRITER_H
#define DOCUMENT_WRITER_H

#include "lucene.h"
#include "lcn_analysis.h"
#include "lcn_search.h"
#include "field_infos.h"

typedef struct lcn_document_writer_t lcn_document_writer_t;
typedef struct lcn_term_vector_offset_info_t lcn_term_vector_offset_info_t;

struct lcn_term_vector_offset_info_t {
    unsigned int start_offset;
    unsigned int end_offset;
};


struct lcn_document_writer_t {

    /**
     * APR pool
     */
    apr_pool_t *pool;

    lcn_directory_t *directory;
    unsigned int max_field_length;
    unsigned int term_index_interval;

    /**
     * This pool is used to create document postings.
     * it is cleared after writing postings to segments
     */
    apr_pool_t *posting_table_pool;

    /**
     * Contains <field_name>:<field_postings> pairs for
     * postings. field_postins are hashes of postings for some
     * field.
     */
    apr_hash_t *posting_table;

    /**
     * Contains field_postings for some field. after processing
     * the field in the invert_document function, this hash is
     * stored in the posting table
     */
    apr_hash_t *field_postings;

    /**
     * Contains the list of postings arrays (lcn_ptr_array_t)
     */
    lcn_ptr_array_t *sorted_postings;

    unsigned int *field_lengths;
    unsigned int *field_positions;
    unsigned int *field_offsets;
    float *field_boosts;

    lcn_field_infos_t *field_infos;
    lcn_term_t *term;

    lcn_similarity_t *similarity;
};

apr_status_t
lcn_document_writer_create( lcn_document_writer_t **document_writer,
                            lcn_directory_t *directory,
                            lcn_index_writer_t *index_writer,
                            apr_pool_t *pool );

apr_status_t
lcn_document_writer_add_document( lcn_document_writer_t *document_writer,
                                  const char *segment_name,
                                  lcn_document_t *document );



#endif /* DOCUMENT_WRITER_H */
