#include "lucene.h"
#include "compound_file_util.h"
#include "lcn_util.h"

/**
 * An array of file extensions for the compound file index.
 */
char* COMPOUND_EXTENSIONS[] = { "fnm", "frq", "prx", "fdx",
                                       "fdt", "tii", "tis", NULL };

lcn_bool_t
lcn_compound_file_util_check_file_extension ( char* filename )
{
    int i = 0;
    lcn_bool_t flag = LCN_FALSE;
    
    for( i = 0; i < CP_EXT_COUNT; i++ )
    {
        char *marker = NULL;
        if ( ( marker = strstr( filename, COMPOUND_EXTENSIONS[i] ) ) )
        {
            if ( strlen( marker ) == strlen( COMPOUND_EXTENSIONS[i] ) )
            {
                flag = LCN_TRUE;
                break;
            }
        }
    }
    
    return flag;
}

apr_status_t
lcn_compound_file_util_compound_file_exists( lcn_directory_t *dir,
                                             const char *file_name,
                                             lcn_bool_t *flag,
                                             apr_pool_t *pool)
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *child_pool;
    char *cfs_name = NULL;
    
    do
    {
        LCNCE(apr_pool_create(&child_pool, pool));
        LCNPV( cfs_name = apr_pstrcat(child_pool, file_name, ".cfs", NULL), LCN_ERR_NULL_PTR);
        LCNCE(lcn_directory_file_exists(dir, cfs_name, flag));
    }
    while(0);
 
    if ( child_pool != NULL )
    {
        apr_pool_destroy(child_pool);
    }
    
    return s;
}
