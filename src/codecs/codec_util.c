#include <lucene.h>
#include <lcn_store.h>
#include "codec.h"

apr_status_t
lcn_codec_util_write_header( lcn_index_output_t *out, 
                             char* codec, 
                             unsigned int version )
{
    apr_status_t s = APR_SUCCESS;
    
    /**
     * Length in bytes, we use ISO
     */
    if ( strlen( codec ) >= 128 )
    {
        return LCN_ERR_CODE_MUSST_BE_SIMPLE_ASCII;
    }
    
    LCNCR( lcn_ostream_write_int( out, LCN_CODEC_MAGIC ) );
    LCNCR( lcn_ostream_write_string( out, codec ) );
    LCNCR( lcn_ostream_write_int( out, version ) );
    
    return s;
}
