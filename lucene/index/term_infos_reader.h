#ifndef TERM_INFOS_READER_H
#define TERM_INFOS_READER_H

#include "lucene.h"
#include "term.h"
#include "field_infos.h"

typedef struct lcn_term_infos_reader_t lcn_term_infos_reader_t;

struct lcn_term_infos_reader_t{

    apr_pool_t *pool;
    lcn_directory_t *dir;
    char *segment;
    lcn_field_infos_t *field_infos;
    lcn_term_enum_t *orig_enum;
    apr_uint64_t size;

    lcn_term_t **index_terms;
    lcn_term_info_t *index_infos;
    apr_off_t *index_pointers;
    unsigned int index_size;

};

apr_status_t
lcn_term_infos_reader_create( lcn_term_infos_reader_t **term_infos_reader,
                              lcn_directory_t *dir,
                              const char *seg,
                              lcn_field_infos_t *field_infos,
                              apr_pool_t *pool );

apr_status_t
lcn_term_infos_reader_terms( lcn_term_infos_reader_t *ti_reader,
                             lcn_term_enum_t **term_enum,
                             apr_pool_t *pool );

/**
 * @param term_info is not allocated, but is set to an internal object
 */
apr_status_t
lcn_term_infos_reader_get_by_term( lcn_term_infos_reader_t *ti_reader,
                                   lcn_term_info_t **term_info,
                                   const lcn_term_t *term );

apr_status_t
lcn_term_infos_reader_terms_from( lcn_term_infos_reader_t *ti_reader,
                                  lcn_term_enum_t **term_enum,
                                  lcn_term_t *term,
                                  apr_pool_t *pool );

apr_status_t
lcn_term_infos_reader_get_term_at_pos( lcn_term_infos_reader_t *ti_reader,
                                       lcn_term_t **term,
                                       apr_off_t pos );

apr_status_t
lcn_term_infos_reader_close( lcn_term_infos_reader_t *ti_reader );

unsigned int
lcn_term_infos_reader_size( lcn_term_infos_reader_t *ti_reader );

unsigned int
lcn_term_infos_reader_skip_interval( lcn_term_infos_reader_t *ti_reader );

apr_status_t
lcn_term_info_create( lcn_term_info_t **term_info,
                      unsigned int doc_freq,
                      apr_off_t freq_pointer,
                      apr_off_t prox_pointer,
                      apr_off_t skip_offset,
                      apr_pool_t *pool );


#endif /* TERM_INFOS_READER_H */
