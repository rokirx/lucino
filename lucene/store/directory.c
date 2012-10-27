#include "lucene.h"
#include "lcn_store.h"
#include "lcn_util.h"

#include "directory.h"
#include "segment_infos.h"
#include "istream.h"
#include "fs_field.h"
#include "compound_file_reader.h"

#if 0

/* How to use lcn_directory_list */

        lcn_list_t *file_list;
        lcn_directory_t *directory = lcn_segment_info_directory( segment_info );

        LCNCE( lcn_directory_list( directory, &file_list, pool ) );

        unsigned int i;

        for ( i = 0; i < lcn_list_size( file_list ); i++ )
        {
            char *s = lcn_list_get( file_list, i );
            fprintf(stderr, "FILE <%s>\n", s );
        }
#endif
        
/**
 * =============================================================================
 *                              Prototypes
 * =============================================================================
 */
        
static apr_status_t
lcn_fs_directory_delete_file ( lcn_directory_t *directory,
                               const char *file_name );

static apr_status_t
lcn_fs_directory_file_exists( const lcn_directory_t *directory,
                              const char *file_name,
                              lcn_bool_t *flag );

static apr_status_t
lcn_fs_directory_list( const lcn_directory_t *directory,
                        lcn_list_t **file_list,
                        apr_pool_t *pool );

/**
 * Oeffnet cfs-Dateien, welche mit dem compound_file_reader verarbeitet werden.
 */
static apr_status_t
lcn_cfs_directory_open_file( lcn_directory_t *directory,
                             lcn_istream_t **new_in,
                             const char *file_name,
                             apr_pool_t *pool );

/**
 * Definiert, ob die gesuchte cfs-Datei existiert. Es wird nicht in der
 * cfs-Datei nach einem Dateinamen gesucht.
 */
static apr_status_t
lcn_cfs_directory_file_exists( const lcn_directory_t *directory,
                               const char *file_name,
                               lcn_bool_t *flag );

/**
 * Noch nicht implementiert.
 */
static apr_status_t
lcn_cfs_directory_create_file( lcn_directory_t *directory,
                               lcn_ostream_t **os,
                               const char *file_name,
                               apr_pool_t *pool );

static apr_status_t
lcn_cfs_directory_list( const lcn_directory_t *directory,
                        lcn_list_t **file_list,
                        apr_pool_t *pool );
static apr_status_t
lcn_cfs_directory_remove( lcn_directory_t *directory );

/**
 * Noch nicht implementiert.
 */
static apr_status_t
lcn_cfs_directory_rename_file( lcn_directory_t *directory,
                               const char *old_name,
                               const char *new_name );

/**
 * =============================================================================
 *                              IMPLEMENTATIONS
 * =============================================================================
 */


/* {{{ apr_status_t lcn_directory_list */

char *
lcn_directory_name( lcn_directory_t *directory,
                    apr_pool_t *pool )
{
    return NULL == pool ? directory->name : apr_pstrdup( pool, directory->name );
}

apr_status_t
lcn_directory_list( const lcn_directory_t *directory,
                    lcn_list_t **file_list,
                    apr_pool_t *pool )
{
    apr_status_t s;
    LCNCR( directory->_list( directory, file_list, pool ) );
    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_segments_format */

apr_status_t
lcn_directory_segments_format( lcn_directory_t *directory,
                               int *format )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *pool = NULL;

    do
    {
        lcn_segment_infos_t *segment_infos;

        if ( directory->segments_format_is_init )
        {
            *format = directory->segments_format;
            break;
        }

        LCNCE( apr_pool_create( &pool, directory->pool ));
        LCNCE( lcn_segment_infos_create( &segment_infos, pool ));

        if ( APR_SUCCESS == lcn_segment_infos_read( segment_infos, directory ) )
        {
            *format = directory->segments_format = lcn_segment_infos_format( segment_infos );
        }
        else
        {
            *format = directory->segments_format = LCN_SEGMENT_INFOS_FORMAT;
        }

        directory->segments_format_is_init = LCN_TRUE;
    }
    while(0);

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_rename_file */

apr_status_t
lcn_directory_rename_file( lcn_directory_t *directory,
                           const char *from,
                           const char *to )
{
    apr_status_t s;
    LCNCR( directory->_rename_file( directory, from, to ) );
    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_open_input */

apr_status_t
lcn_directory_open_input( lcn_directory_t *directory,
                          lcn_istream_t **new_in,
                          const char *file_name,
                          apr_pool_t *pool )
{
    apr_status_t s;

    /* opening non-existing segments while indexing is a common error */

    if ( 0 == strcmp( "segments", file_name ) )
    {
        return directory->_open_file( directory, new_in, file_name, pool );
    }
    
    LCNRM( directory->_open_file( directory, new_in, file_name, pool ), file_name );

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_delete_file */

apr_status_t
lcn_directory_delete_file( lcn_directory_t *directory,
                           const char *file_name )
{
    apr_status_t s;
    LCNRM( directory->_delete_file( directory, file_name ), file_name );
    return s;
}

apr_status_t
lcn_directory_delete_files( lcn_directory_t *directory,
                            lcn_list_t *list_file_names )
{
    apr_status_t s;
    int i = 0;
    for( i = 0; i < lcn_list_size(list_file_names); i++ )
    {
        char *file_name = lcn_list_get(list_file_names, i);
        LCNRM( directory->_delete_file( directory, file_name ), file_name );
    }
    
    return s;
}

/* }}} */

apr_status_t
lcn_directory_remove( lcn_directory_t *directory )
{
    apr_status_t s;
    LCNCR( directory->_remove( directory ));
    return s;
}

/* {{{ apr_status_t lcn_directory_create_output */

apr_status_t
lcn_directory_create_output ( lcn_directory_t *directory,
                              lcn_ostream_t **os,
                              const char *file_name,
                              apr_pool_t *pool )
{
    apr_status_t s;
    LCNCR( directory->_create_file( directory, os, file_name, pool ) );
    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_directory_open_ostream */

static apr_status_t
lcn_directory_open_ostream ( const char *path,
                             const char *name,
                             lcn_ostream_t **os,
                             apr_pool_t *pool )
{
    apr_status_t s;
    char *fpath;

    LCNCR( apr_filepath_merge( &fpath, path, name, 0, pool ) );
    LCNCR( lcn_fs_ostream_create( os, fpath, pool ) );

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_directory_open_input_stream */

static apr_status_t
lcn_directory_open_input_stream ( const char *path,
                                  const char *name,
                                  lcn_istream_t **is,
                                  apr_pool_t *pool )
{
    apr_status_t s;
    char *fpath;

    LCNCR( apr_filepath_merge( &fpath, path, name, 0, pool ) );
    LCNRM( lcn_istream_create( is, fpath, pool ), fpath );

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_close */

apr_status_t
lcn_directory_close( lcn_directory_t *directory )
{
    directory->is_open = LCN_FALSE;
    return APR_SUCCESS;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_open_file */

static apr_status_t
lcn_fs_directory_open_file( lcn_directory_t *directory,
                            lcn_istream_t **new_in,
                            const char *file_name,
                            apr_pool_t *pool )
{
    apr_status_t s;
    LCNRM( lcn_directory_open_input_stream( directory->name,
                                            file_name,
                                            new_in,
                                            pool ),
                                            file_name );
    return s;
}

static apr_status_t
lcn_cfs_directory_open_file( lcn_directory_t *directory,
                             lcn_istream_t **new_in,
                             const char *file_name,
                             apr_pool_t *pool )
{
    
    apr_status_t s = APR_SUCCESS;
    
    do
    {
        if ( 0 == strcmp(file_name, "segments") )
        {
            LCNCE(lcn_fs_directory_open_file(directory, new_in, file_name, pool));
        }
        else
        {
            LCNCE(lcn_compound_file_reader_open_input(directory->cfr, new_in, file_name));
        }
        
    }
    while(0);
    
    return s;
}

static apr_status_t
lcn_cfs_directory_create_file( lcn_directory_t *directory,
                               lcn_ostream_t **os,
                               const char *file_name,
                               apr_pool_t *pool )
{
    fprintf(stderr, "TODO: lucene/store/directory.c -> lcn_cfs_directory_create_file not implemented\n");
    return LCN_ERR_METHOD_NOT_IMPLEMENTED;
}

static apr_status_t
lcn_cfs_directory_delete_file( lcn_directory_t *directory,
                               const char *file_name )
{
    fprintf(stderr, "TODO: lucene/store/directory.c -> lcn_cfs_directory_delete_file not implemented\n");
    return LCN_ERR_METHOD_NOT_IMPLEMENTED;
}

static apr_status_t
lcn_cfs_directory_file_exists( const lcn_directory_t *directory,
                               const char *file_name,
                               lcn_bool_t *flag )
{
    *flag = lcn_compound_file_reader_file_name_exists(directory->cfr, file_name);    
    return APR_SUCCESS;
    
}

static apr_status_t
lcn_cfs_directory_rename_file( lcn_directory_t *directory,
                               const char *old_name,
                               const char *new_name )
{
    fprintf(stderr, "TODO: lucene/store/directory.c -> lcn_cfs_directory_rename_file not implemented\n");
    return LCN_ERR_METHOD_NOT_IMPLEMENTED;
}

static apr_status_t
lcn_cfs_directory_list( const lcn_directory_t *directory,
                        lcn_list_t **file_list,
                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        printf("\n\n lcn_cfs_directory_list call \?");
        LCNCE(lcn_compound_file_reader_entries_as_list(directory->cfr, file_list, pool));
        unsigned int i = 0, j = lcn_list_size(*file_list);
        for(i = 0; i < j; ++i)
        {
            printf("\n\nname: %s", (char*)lcn_list_get(*file_list, i)); 
        }
    }
    while(0);
    
    return s;
}

static apr_status_t
lcn_cfs_directory_remove( lcn_directory_t *directory )
{
    fprintf(stderr, "TODO: lucene/store/directory.c -> lcn_cfs_directory_remove not implemented\n");
    return LCN_ERR_METHOD_NOT_IMPLEMENTED;
}

apr_status_t 
lcn_cfs_directory_create( lcn_directory_t **new_dir,
                          lcn_directory_t *cf_base_dir,
                          const char *cf_name,
                          apr_pool_t *pool)
{
    apr_status_t s;

    do
    {
        /* initialize type struct */
        LCNCE( lcn_base_directory_create( new_dir, pool ) );
        LCNPV( (*new_dir)->name = apr_pstrdup( pool, cf_base_dir->name ), APR_ENOMEM );
        
        /* Set compound file informations */
        lcn_compound_file_reader_t *cfr;
        LCNCE( lcn_compound_file_reader_create(&cfr, cf_base_dir, cf_name, pool ));
        (*new_dir)->cfr = cfr;
        
        /*(*new_dir)->cf_name = cf_name;
        (*new_dir)->cf_count = cf_count;
        (*new_dir)->cf_entries = cf_entries;
        (*new_dir)->cf_stream = cf_stream;*/
        
        (*new_dir)->_open_file   = lcn_cfs_directory_open_file;
        (*new_dir)->_create_file = lcn_cfs_directory_create_file;
        (*new_dir)->_delete_file = lcn_cfs_directory_delete_file;
        (*new_dir)->_file_exists = lcn_cfs_directory_file_exists;
        (*new_dir)->_rename_file = lcn_cfs_directory_rename_file;
        (*new_dir)->_list        = lcn_cfs_directory_list;
        (*new_dir)->_remove      = lcn_cfs_directory_remove;
        
        cfr->dir = (*new_dir);
    }
    while(0);

    /* clean up on error */
    if (s)
    {
        *new_dir = NULL;
    }

    return s;
}



















static apr_status_t
lcn_fs_directory_remove( lcn_directory_t *directory )
{
    apr_status_t s;

    do
    {
        LCNCE( apr_dir_remove( directory->name, directory->pool ));
        LCNCE( lcn_directory_close( directory ));
    }
    while(0);

    return s;
}

static apr_status_t
lcn_ram_directory_remove( lcn_directory_t *directory )
{
    return APR_SUCCESS;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_open_segment_file */

apr_status_t
lcn_directory_open_segment_file ( lcn_directory_t *directory,
                                  lcn_istream_t **new_in,
                                  const char *seg_name,
                                  const char *ext,
                                  apr_pool_t *pool )
{
    apr_status_t s;
    char *fname;

    do
    {
        LCNPV( fname = apr_pstrcat( pool, seg_name, ext, NULL ), APR_ENOMEM );
        LCNCE( lcn_directory_open_input( directory, new_in, fname, pool ) );
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_create_segment_file */

apr_status_t
lcn_directory_create_segment_file ( lcn_directory_t *directory,
                                    lcn_ostream_t **ostream,
                                    const char *seg_name,
                                    const char *ext,
                                    apr_pool_t *pool )
{
    apr_status_t s;
    char *fname;

    do
    {
        LCNPV( fname = apr_pstrcat( pool, seg_name, ext, NULL ), APR_ENOMEM );
        LCNCM( lcn_directory_create_output( directory, ostream, fname, pool ), fname );
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_file_exists */

apr_status_t
lcn_file_exists( const char *file_name, lcn_bool_t *flag, apr_pool_t *pool )
{
    apr_finfo_t finfo;
    apr_status_t s;

    s = apr_stat( &finfo, file_name, APR_FINFO_TYPE, pool );

    if ( s == APR_ENOENT || s == 720002 || s == 720003 )
    {
        *flag = LCN_FALSE;
        return APR_SUCCESS;
    }

    if ( s == APR_SUCCESS )
    {
        *flag = LCN_TRUE;
    }

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_directory_file_exists */

apr_status_t
lcn_directory_file_exists ( const lcn_directory_t *directory,
                            const char *file_name,
                            lcn_bool_t *flag )
{
    apr_status_t s;
    LCNCR( directory->_file_exists( directory, file_name, flag ) );
    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_file_exists */

static apr_status_t
lcn_fs_directory_file_exists( const lcn_directory_t *directory,
                              const char *file_name,
                              lcn_bool_t *flag )
{
    apr_status_t s;
    apr_pool_t *pool;

    if ( APR_SUCCESS != ( s = apr_pool_create( &pool, directory->pool ) ) )
    {
        return s;
    }

    do
    {
        char *path;
        LCNCE( apr_filepath_merge( &path, directory->name, file_name, 0, pool ) );
        LCNCE( lcn_file_exists( path, flag, pool ) );
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_list */

static apr_status_t
lcn_fs_directory_list( const lcn_directory_t *directory,
                       lcn_list_t **file_list,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_dir_t *dir;
        apr_finfo_t finfo;
        apr_status_t dir_read;

        LCNCE( lcn_list_create( file_list, 10, pool ));
        LCNCE( apr_dir_open( &dir, directory->name, pool) );

        while( APR_SUCCESS == ( dir_read = apr_dir_read( &finfo, APR_FINFO_NAME | APR_FINFO_TYPE, dir )))
        {

            if ( finfo.filetype == APR_LNK )
            {
                /* TODO */
            }
            else if ( finfo.filetype == APR_REG )
            {
                char *name;

                LCNPV( name = apr_pstrdup( pool, finfo.name ), APR_ENOMEM );
                LCNCE( lcn_list_add( *file_list, name ) );
            }
        }

        LCNCE( apr_dir_close( dir ));
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_create_file */

static apr_status_t
lcn_fs_directory_create_file ( lcn_directory_t *directory,
                               lcn_ostream_t **os,
                               const char *file_name,
                               apr_pool_t *pool )
{
    apr_status_t s;
    LCNCR( lcn_directory_open_ostream ( directory->name, file_name, os, pool ) );
    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_rename_file */

static apr_status_t
lcn_fs_directory_rename_file ( lcn_directory_t *directory,
                               const char *old_name,
                               const char *new_name )
{
    apr_status_t s;
    apr_pool_t *pool;

    if (( s = apr_pool_create( &pool, directory->pool )))
    {
        return s;
    }

    do
    {
        char *full_old_name, *full_new_name;

        LCNCE( apr_filepath_merge( &full_old_name, directory->name, old_name, 0, pool ) );
        LCNCE( apr_filepath_merge( &full_new_name, directory->name, new_name, 0, pool ) );
        LCNCE( apr_file_rename( full_old_name, full_new_name, pool ) );
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_fs_directory_delete_file */

static apr_status_t
lcn_fs_directory_delete_file ( lcn_directory_t *directory,
                               const char *file_name )
{
    apr_status_t s;
    apr_pool_t *pool;
    char *path;
    lcn_bool_t flag;

    if ( APR_SUCCESS != ( s = apr_pool_create( &pool, directory->pool ) ) )
    {
        return s;
    }

    do
    {
        LCNCE( apr_filepath_merge( &path, directory->name, file_name, 0, pool ) );
        LCNCE( lcn_file_exists( path, &flag, pool ) );

        if ( flag == LCN_TRUE )
        {
            LCNCE( apr_file_remove( path, pool ) );
        }
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_base_directory_create */

apr_status_t
lcn_base_directory_create ( lcn_directory_t **directory,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *directory = (lcn_directory_t *) apr_pcalloc( pool, sizeof(lcn_directory_t) ), APR_ENOMEM );
        (*directory)->pool    = pool;
        (*directory)->is_open = LCN_TRUE;
        (*directory)->_close  = lcn_directory_close;
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_fs_directory_create */

apr_status_t
lcn_fs_directory_create( lcn_directory_t **new_dir,
                         const char *dir_name,
                         lcn_bool_t create,
                         apr_pool_t *pool )
{
    apr_status_t s;
    apr_finfo_t finfo;
    apr_dir_t* dir = NULL;
    apr_pool_t *child_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &child_pool, pool ));

        s = apr_stat( &finfo, dir_name, APR_FINFO_TYPE, child_pool );

        /* create directory if there does not exist one */

        if ( APR_ENOENT == s || 720002 == s || 720003 == s)
        {
            if (create == LCN_TRUE )
            {
                LCNCE( apr_dir_make_recursive( dir_name, APR_OS_DEFAULT, child_pool ) );
            }
            else
            {
                s = LCN_ERR_CANNOT_OPEN_DIR;
                break;
            }
        }
        else
        {
            if ( finfo.filetype != APR_DIR )
            {
                return LCN_ERR_CANNOT_OPEN_DIR;
            }
        }

        /* check whether we can open/close directory */

        LCNCM( apr_dir_open( &dir, dir_name, pool ), dir_name );
        LCNCE( apr_dir_close( dir ) );

        /* initialize type struct */
        LCNCE( lcn_base_directory_create( new_dir, pool ) );
        LCNPV( (*new_dir)->name = apr_pstrdup( pool, dir_name ), APR_ENOMEM );

        (*new_dir)->_open_file   = lcn_fs_directory_open_file;
        (*new_dir)->_create_file = lcn_fs_directory_create_file;
        (*new_dir)->_delete_file = lcn_fs_directory_delete_file;
        (*new_dir)->_file_exists = lcn_fs_directory_file_exists;
        (*new_dir)->_rename_file = lcn_fs_directory_rename_file;
        (*new_dir)->_list        = lcn_fs_directory_list;
        (*new_dir)->_remove      = lcn_fs_directory_remove;
    }
    while(0);

    /* clean up on error */
    if (s)
    {
        if ( child_pool != NULL )
        {
            apr_pool_destroy( child_pool );
        }

        *new_dir = NULL;
    }

    return s;
}

/* }}} */

/* {{{ lcn_bool_t lcn_directory_is_open */

lcn_bool_t
lcn_directory_is_open( lcn_directory_t *directory )
{
    return directory->is_open;
}

/* }}} */

static char *NAME = "RAM_DIR";

/* {{{ apr_status_t lcn_ram_file_create */

apr_status_t
lcn_ram_file_create ( lcn_ram_file_t **ram_file,
                      apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *p = NULL;

    do
    {
        LCNCE( apr_pool_create( &p, pool ) );
        LCNPV( *ram_file = (lcn_ram_file_t*) apr_palloc( p, sizeof(lcn_ram_file_t) ), APR_ENOMEM );
        LCNCE( lcn_list_create( &((*ram_file)->buffers), 4, p ) );

        (*ram_file)->pool = p;
        (*ram_file)->length = 0;
    }
    while(0);

    if ( s && p != NULL )
    {
        apr_pool_destroy( p );
    }

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_list */

static apr_status_t
lcn_ram_directory_list( const lcn_directory_t *directory,
                        lcn_list_t **file_list,
                        apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_hash_index_t *hi;

        LCNCE( lcn_list_create( file_list, 10, pool ));

        for( hi = apr_hash_first( pool, directory->hash); hi; hi = apr_hash_next( hi ))
        {
            char *name;
            const void *key;

            apr_hash_this( hi, &key, NULL, NULL );

            LCNPV( name = apr_pstrdup( pool, (char*) key ), APR_ENOMEM );
            LCNCE( lcn_list_add( *file_list, name ) );
        }
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_create_file */

static apr_status_t
lcn_ram_directory_create_file ( lcn_directory_t *directory,
                                lcn_ostream_t **new_os,
                                const char *file_name,
                                apr_pool_t *pool )
{
    apr_status_t s;
    char* fname_dup;
    lcn_ram_file_t *file;

    do
    {
        file = (lcn_ram_file_t *) apr_hash_get( directory->hash, file_name, strlen(file_name) );

        if ( NULL == file )
        {
            LCNCE( lcn_ram_file_create( &file, directory->pool ) );

            /**
             * WARNING: it may be dangerous to let file own its name. We assume
             * here lifetime of file name == lifetime of file
             */
            LCNPV( fname_dup = apr_pstrdup( file->pool, file_name ), APR_ENOMEM );
            apr_hash_set( directory->hash, fname_dup, strlen(fname_dup), file );
        }

        LCNCE( lcn_ram_ostream_create( new_os, file, pool ) );
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_rename_file */

static apr_status_t
lcn_ram_directory_rename_file( lcn_directory_t *directory,
                               const char *from,
                               const char *to )
{
    apr_status_t s;
    lcn_ram_file_t *file;

    do
    {
        LCNPV( file = (lcn_ram_file_t *) apr_hash_get( directory->hash, from, strlen(from) ),
               LCN_ERR_RAM_FILE_NOT_FOUND );
        apr_hash_set( directory->hash, to, strlen(to), file );
        apr_hash_set( directory->hash, from, strlen(from), NULL );
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_open_file */

static apr_status_t
lcn_ram_directory_open_file ( lcn_directory_t *directory,
                              lcn_istream_t **new_in,
                              const char *file_name,
                              apr_pool_t *pool )
{
    apr_status_t s;
    lcn_ram_file_t *file;

    do
    {
        file = (lcn_ram_file_t *) apr_hash_get( directory->hash, file_name, strlen(file_name) );

        if ( 0 == strcmp( "segments", file_name ) && NULL == file )
        {
            return LCN_ERR_RAM_FILE_NOT_FOUND;
        }

        LCNPV( file, LCN_ERR_RAM_FILE_NOT_FOUND );
        LCNCM( lcn_ram_input_stream_create( new_in, file, pool ), file_name );
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_file_exists */

static apr_status_t
lcn_ram_directory_file_exists( const lcn_directory_t *directory,
                               const char *file_name,
                               lcn_bool_t *file_exists )
{
    if ( NULL != ( (lcn_ram_file_t *) apr_hash_get( directory->hash,
                                                    file_name,
                                                    strlen(file_name) ) ) )
    {
        *file_exists = LCN_TRUE;
    }
    else
    {
        *file_exists = LCN_FALSE;
    }

    return APR_SUCCESS;
}

/* }}} */

/* {{{ static apr_status_t lcn_ram_directory_delete_file */

static apr_status_t
lcn_ram_directory_delete_file( lcn_directory_t *directory,
                               const char *file_name )
{
    lcn_ram_file_t *file = (lcn_ram_file_t *) apr_hash_get( directory->hash, file_name, strlen(file_name) );

    if ( NULL != file )
    {
        apr_hash_set( directory->hash, file_name, strlen(file_name), NULL );
        apr_pool_destroy( file->pool );
    }

    return APR_SUCCESS;
}

/* }}} */

/* {{{ apr_status_t lcn_ram_directory_create */

apr_status_t
lcn_ram_directory_create( lcn_directory_t **new_dir, apr_pool_t *pool )
{
    apr_status_t s;

    if ((s = lcn_base_directory_create( new_dir, pool )))
    {
        return s;
    }

    (*new_dir)->hash = apr_hash_make((*new_dir)->pool );

    (*new_dir)->name         = NAME;
    (*new_dir)->_open_file   = lcn_ram_directory_open_file;
    (*new_dir)->_create_file = lcn_ram_directory_create_file;
    (*new_dir)->_delete_file = lcn_ram_directory_delete_file;
    (*new_dir)->_file_exists = lcn_ram_directory_file_exists;
    (*new_dir)->_rename_file = lcn_ram_directory_rename_file;
    (*new_dir)->_list        = lcn_ram_directory_list;
    (*new_dir)->_remove      = lcn_ram_directory_remove;

    return APR_SUCCESS;
}

/* }}} */
