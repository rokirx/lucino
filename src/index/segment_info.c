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

apr_status_t
lcn_segment_info_to_string( char** str,
                            lcn_segment_info_t *segment_info,
                            lcn_directory_t *dir,
                            unsigned int del_count,
                            apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    do
    {
        lcn_string_buffer_t *sb;
        const char* cfs = segment_info->is_compound_file ? "c" : "C";

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_string_buffer_create( &sb, cp ) );

        LCNCE( lcn_string_buffer_append( sb, segment_info->name ) );
        LCNCE( lcn_string_buffer_append( sb, "(" ) );
        LCNCE( lcn_string_buffer_append( sb, ( segment_info->version == NULL ? "?" : segment_info->version) ) );
        LCNCE( lcn_string_buffer_append( sb, "):" ) );
        LCNCE( lcn_string_buffer_append( sb, cfs ) );

        if( segment_info->directory != dir )
        {
            LCNCE( lcn_string_buffer_append( sb, "x" ) )   ;
        }

        if( del_count != 0 )
        {
            LCNCE( lcn_string_buffer_append( sb, "/" ) );
            LCNCE( lcn_string_buffer_append( sb, apr_itoa( cp, del_count ) ) );
        }

        // TODO: we could append toString of attributes() here?

        LCNCE( lcn_string_buffer_to_string( sb, str, pool ) );
    }
    while(0);

    if( cp == NULL )
    {
        apr_pool_destroy( cp );
    }

    fprintf(stderr, "\n to_string: %s \n ", (*str));
    exit(0);

    return s;
}
