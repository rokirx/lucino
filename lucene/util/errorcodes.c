#include <apr_strings.h>
#include <apr_lib.h>
#include "lucene.h"

BEGIN_C_DECLS

typedef struct {
    lcn_errno_t errcode;
    const char *errdesc;
} err_defn;

#define LCN_ERROR_BUILD_ARRAY
#include "lcn_error_codes.h"

char *
lcn_strerror ( apr_status_t statcode, char *buf, unsigned int bufsize)
{
    const err_defn *defn;

    for ( defn = error_table; defn->errdesc != NULL; defn++)
    {
        if ( defn->errcode == (lcn_errno_t) statcode )
        {
            apr_cpystrn ( buf, defn->errdesc, bufsize);
            return buf;
        }
    }

    return apr_strerror (statcode, buf, bufsize);
}


END_C_DECLS
