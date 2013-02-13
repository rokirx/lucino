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

apr_status_t
lcn_segment_info_per_commit_to_string ( char** str,
                                        lcn_segment_info_per_commit_t *info_pc,
                                        lcn_directory_t *directory,
                                        unsigned int pending_del_count,
                                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    LCNCR( lcn_segment_info_to_string( str,
                                       info_pc->segment_info,
                                       directory,
                                       pending_del_count,
                                       pool ) );


    return s;
}

char*
lcn_segment_info_per_commit_to_hash( lcn_segment_info_per_commit_t *info_pc,
                                     apr_pool_t *pool)
{
    return apr_pstrcat( pool,
                        info_pc->segment_info->directory->name,
                        info_pc->segment_info->name,
                        NULL);
}
