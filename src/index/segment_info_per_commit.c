#include "segment_infos.h"
#include "directory.h"

/********************************************************
 *                                                      *
 * Functions of segment_info_per_commit                 *
 *                                                      *
 ********************************************************/

lcn_segment_info_t *
lcn_segment_info_per_commit_info( lcn_segment_info_per_commit_t *info_pc )
{
    return info_pc->segment_info;
}

