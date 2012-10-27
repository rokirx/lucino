#ifndef SEGMENT_TERM_ENUM_H
#define SEGMENT_TERM_ENUM_H

#include "lcn_util.h"

apr_status_t
lcn_term_enum_clone( lcn_term_enum_t *term_enum,
                     lcn_term_enum_t **clone,
                     apr_pool_t *pool );

apr_status_t
lcn_term_enum_seek( lcn_term_enum_t *term_enum,
                    apr_off_t pointer,
                    apr_off_t p,
                    lcn_term_t *term,
                    lcn_term_info_t *ti );

apr_status_t
lcn_term_enum_scan_to( lcn_term_enum_t *term_enum,
                       lcn_term_t *term );

apr_status_t
lcn_term_enum_close( lcn_term_enum_t *term_enum );


#endif
