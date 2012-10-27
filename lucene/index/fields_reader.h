#ifndef FIELDS_READER_H
#define FIELDS_READER_H

#include "lucene.h"
#include "apr_pools.h"
#include "field_infos.h"

typedef struct lcn_fields_reader_t lcn_fields_reader_t;

struct lcn_fields_reader_t {

    unsigned char flags;

    lcn_field_infos_t *field_infos;
    lcn_istream_t *fields_stream;
    lcn_istream_t *index_stream;

    int format;

    unsigned int size;
};

apr_status_t
lcn_fields_reader_create( lcn_fields_reader_t **fields_reader,
                          lcn_directory_t *directory,
                          const char *segment,
                          lcn_field_infos_t *field_infos,
                          apr_pool_t *pool );

apr_status_t
lcn_fields_reader_close( lcn_fields_reader_t *fields_reader );

unsigned int
lcn_fields_reader_size( lcn_fields_reader_t *fields_reader );

apr_status_t
lcn_fields_reader_doc( lcn_fields_reader_t *fields_reader,
                       lcn_document_t **document,
                       unsigned int n,
                       apr_pool_t *pool );

#endif
