#include "segment_infos.h"
#include "directory.h"

/********************************************************
 *                                                      *
 * Functions of segment_info                            *
 *                                                      *
 * constructor is inlined in lcn_segment_infos_add_info *
 *                                                      *
 ********************************************************/

const char *
lcn_segment_info_name( lcn_segment_info_t *segment_info )
{
    return segment_info->name;
}

lcn_directory_t *
lcn_segment_info_directory( lcn_segment_info_t *segment_info )
{
    return segment_info->directory;
}

unsigned int
lcn_segment_info_doc_count( lcn_segment_info_t *segment_info )
{
    return segment_info->doc_count;
}

apr_status_t
lcn_segment_info_has_deletions( lcn_segment_info_t *segment_info,
                                lcn_bool_t *has_deletions )
{
    apr_status_t s;
    char *del_file;
    apr_pool_t *pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &pool, segment_info->directory->pool ));
        LCNPV( del_file = apr_pstrcat( pool, segment_info->name, ".del", NULL ), APR_ENOMEM );
        LCNCE( lcn_directory_file_exists ( segment_info->directory, del_file, has_deletions ) );
    }
    while(0);

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}
