#ifndef FIELDS_WRITER_H
#define FIELDS_WRITER_H

#include "lucene.h"
#include "apr_pools.h"
#include "field_infos.h"

typedef struct lcn_fields_writer_t lcn_fields_writer_t;

struct lcn_fields_writer_t {

    unsigned char flags;

    lcn_field_infos_t   *field_infos;
    lcn_ostream_t *fields_stream;
    lcn_ostream_t *index_stream;

    /* streams for fixed sized fields */

    apr_hash_t* fs_streams;
    unsigned int doc_count;
    lcn_directory_t *directory;
    char *segment;
    apr_pool_t *pool;
};

apr_status_t
lcn_fields_writer_create( lcn_fields_writer_t **fields_writer,
                          lcn_directory_t *directory,
                          const char *segment,
                          lcn_field_infos_t *f_infos,
                          apr_pool_t *pool );

apr_status_t
lcn_fields_writer_close( lcn_fields_writer_t *fields_writer );

apr_status_t
lcn_fields_writer_add_document( lcn_fields_writer_t *fields_writer,
                                lcn_document_t *doc );


#endif
