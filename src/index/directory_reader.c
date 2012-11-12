#include "directory_reader.h"

apr_status_t
lcn_directory_reader_index_exists( lcn_directory_t *directory,
                                   lcn_bool_t *exists,
                                   apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_segment_infos_t *segment_infos;

        LCNCE( lcn_segment_infos_create( &segment_infos, pool ));

        s = lcn_segment_infos_read_directory( segment_infos, directory );
    }
    while(0);
#if 0
  //apr_status_t s = lcn_segment_infos_read_directory( directory );
    try {
      new SegmentInfos().read(directory);
      return true;
    } catch (IOException ioe) {
      return false;
    }
#endif
    *exists = LCN_FALSE;

    return s;
}



