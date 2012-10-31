#include "lcn_util.h"
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

/********************************************************
 *                                                      *
 * Functions of segment_infos                           *
 *                                                      *
 * read current version not implemented. is a conv.     *
 * function                                             *
 *                                                      *
 ********************************************************/

apr_status_t
lcn_segment_infos_create( lcn_segment_infos_t **segment_infos,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *segment_infos = apr_pcalloc( pool, sizeof(lcn_segment_infos_t)), APR_ENOMEM );
        LCNCE( apr_pool_create( &((*segment_infos)->subpool), pool ) );
        LCNCE( lcn_list_create( &((*segment_infos)->list), 10, (*segment_infos)->subpool ) );
        (*segment_infos)->pool = pool;
    }
    while(0);

    return s;
}

void
lcn_segment_infos_remove( lcn_segment_infos_t *segment_infos, unsigned int i )
{
    lcn_list_remove( segment_infos->list, i );
}

unsigned int
lcn_segment_infos_size( lcn_segment_infos_t *segment_infos )
{
    return lcn_list_size( segment_infos->list );
}

apr_uint64_t
lcn_segment_infos_version( lcn_segment_infos_t *segment_infos )
{
    return segment_infos->version;
}

int
lcn_segment_infos_format( lcn_segment_infos_t *segment_infos )
{
    return segment_infos->format;
}

apr_status_t
lcn_segment_infos_get_next_name( lcn_segment_infos_t *segment_infos,
                                 char **seg_name,
                                 apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        char buf[10];
        buf[0] = '_';
        lcn_itoa36( segment_infos->counter, buf + 1 );
        LCNPV( *seg_name = apr_pstrdup( pool, buf ), APR_ENOMEM );
        segment_infos->counter++;
    }
    while(0);

    return s;
}

apr_status_t
lcn_segment_infos_get( lcn_segment_infos_t *segment_infos,
                       lcn_segment_info_t **segment_info,
                       unsigned int nth )
{
    *segment_info = lcn_list_get( segment_infos->list, nth );
    return APR_SUCCESS;
}

apr_status_t
lcn_segment_infos_add_info ( lcn_segment_infos_t *segment_infos,
                             lcn_directory_t *directory,
                             const char *name,
                             unsigned int count )
{
    apr_status_t s;

    do
    {
        lcn_segment_info_t *segment_info;
        apr_pool_t *pool = lcn_list_pool( segment_infos->list );

        LCNPV( segment_info = (lcn_segment_info_t*) apr_pcalloc( pool, sizeof(lcn_segment_info_t) ),
               APR_ENOMEM );

        LCNCE( lcn_list_add( segment_infos->list, segment_info ) );
        LCNPV( segment_info->name = apr_pstrdup( pool, name ), APR_ENOMEM );

        segment_info->doc_count = count;
        segment_info->directory = directory;

        if ( 0 == (segment_infos->counter % 1000))
        {
            unsigned int lsize = lcn_list_size( segment_infos->list );
            unsigned int i;
            lcn_list_t *list;
            apr_pool_t *p;

            LCNCE( apr_pool_create( &p, segment_infos->pool ));
            LCNCE( lcn_list_create( &list, 100, p ));

            for (i = 0; i < lsize; i++ )
            {
                lcn_segment_info_t *si = (lcn_segment_info_t*) lcn_list_get( segment_infos->list, i );
                lcn_segment_info_t *new_si;

                LCNPV( new_si = (lcn_segment_info_t*) apr_pcalloc( p, sizeof(lcn_segment_info_t) ),
                       APR_ENOMEM );

                LCNCE( lcn_list_add( list, new_si ) );
                LCNPV( new_si->name = apr_pstrdup( p, si->name ), APR_ENOMEM );

                new_si->doc_count = si->doc_count;
                new_si->directory = si->directory;
            }

            if ( s )
            {
                break;
            }

            apr_pool_destroy( segment_infos->subpool );
            segment_infos->subpool = p;
            segment_infos->list = list;
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_segment_infos_write( lcn_segment_infos_t *segment_infos,
                         lcn_directory_t *dir )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;
    lcn_ostream_t *os = NULL;

    do
    {

        unsigned int i = 0;
        unsigned int size = lcn_segment_infos_size( segment_infos );

        LCNCE( apr_pool_create( &pool, dir->pool ));
        LCNCE( lcn_directory_create_output( dir, &os, "segments.new", pool ) );
        LCNCE( lcn_ostream_write_int( os, LCN_SEGMENT_INFOS_FORMAT ));
        LCNCE( lcn_ostream_write_long( os, ++(segment_infos->version) ));
        LCNCE( lcn_ostream_write_int( os, segment_infos->counter ));
        LCNCE( lcn_ostream_write_int( os, size ));

        for( i = 0; i < size; i++ )
        {
            lcn_segment_info_t *info;
            LCNCE( lcn_segment_infos_get( segment_infos, &info, i ) );
            LCNCE( lcn_ostream_write_string( os, info->name ) );
            LCNCE( lcn_ostream_write_int( os, info->doc_count ));
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_ostream_close( os ) );
        os = NULL;

        LCNCE( lcn_directory_rename_file( dir, "segments.new", "segments" ) );
    }
    while(0);

    if ( NULL != os )
    {
        (void) lcn_ostream_close( os );
    }

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

apr_status_t
lcn_segment_infos_read( lcn_segment_infos_t *segment_infos,
                        lcn_directory_t *dir )
{
    apr_status_t s;
    apr_pool_t *pool;
    lcn_istream_t *is = NULL;

    do
    {
        int format, size, i;

        LCNCE( apr_pool_create( &pool, segment_infos->pool ));

        if (( s = lcn_directory_open_input( dir, &is, "segments", pool )))
        {
            break;
        }

        LCNCE( lcn_istream_read_int( is, &format ) );

        if ( format < 0 )  /* file contains explicit format info */
        {
            int counter;
            LCNASSERT( format >= LCN_SEGMENT_INFOS_FORMAT, LCN_ERR_SEGMENT_INFOS_UNKNOWN_FILE_FORMAT );
            LCNCE( lcn_istream_read_ulong( is, &(segment_infos->version) ));
            LCNCE( lcn_istream_read_int( is, &counter ) );
            segment_infos->counter = counter;

            segment_infos->format = format;
        }
        else   /* file is in old format without explicit format info */
        {
            segment_infos->format = 0;
            segment_infos->version = 0;
            segment_infos->counter = format;
        }

        LCNCE( lcn_istream_read_int( is, (int*)&size ));

        for( i = 0; i < size; i++ )
        {
            char *name;
            int doc_count;
            unsigned int len;

            LCNCE( lcn_istream_read_string( is, &name, &len, lcn_list_pool( segment_infos->list )));
            LCNCE( lcn_istream_read_int( is, (int*)&doc_count ) );
            LCNCE( lcn_segment_infos_add_info( segment_infos, dir, name, doc_count ));
        }

        dir->segments_format = segment_infos->format;
        dir->segments_format_is_init = LCN_TRUE;

    }
    while(0);

    if ( NULL != is )
    {
        apr_status_t stat = lcn_istream_close( is );
        s = ( s ? s : stat );
    }

    apr_pool_destroy( pool );

    return s;
}

apr_status_t
lcn_segment_infos_has_separate_norms( lcn_segment_info_t *segment_info,
                                      lcn_bool_t *flag,
                                      apr_pool_t *pool)
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *child_pool;
    *flag = LCN_FALSE;
    
    do
    {        
        lcn_list_t *file_list;
        char *pattern;
        int i;
        
        LCNCE( apr_pool_create( &child_pool, pool ));
        
        LCNCE( lcn_directory_list(segment_info->directory, &file_list, child_pool) );
        LCNPV( pattern = apr_pstrcat(child_pool, segment_info->name, ".s", NULL ), LCN_ERR_NULL_PTR);
        
        unsigned int pattern_len = strlen(pattern);
        for ( i = 0; i < lcn_list_size( file_list ); i++ )
        {
            char *file_name = lcn_list_get( file_list, i );
            if( lcn_string_starts_with(file_name,pattern)  && 
                isdigit( file_name[pattern_len] ) )
            {
                *flag = LCN_TRUE;
            }
        }
    }
    while(0);
    
    if ( NULL != child_pool )
    {
        apr_pool_destroy( child_pool );
    }
    
    return s;
}


/**
 * Lucene 4.0
 */
apr_status_t
lcn_segment_infos_read_directory( lcn_segment_infos_t *segment_infos,
                                  lcn_directory_t *directory )
{
}
