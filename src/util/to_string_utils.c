#include "lcn_util.h"

void
lcn_to_string_boost( char* buf, unsigned int buf_len, float boost )
{
    if( boost == 1.0f )
    {
        *buf = 0;
    }
    else
    {
        apr_snprintf( buf, buf_len, "^%f", boost );
    }

}
