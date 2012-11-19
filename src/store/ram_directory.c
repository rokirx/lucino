#include "directory.h"
#include "index_input.h"

static char *NAME = "RAM_DIR";

struct ram_directory
{
    lcn_directory_t directory;

    /**
     * Map<String,RAMFile>
     */
    apr_hash_t *file_map;
};


static apr_status_t
lcn_ram_directory_remove( lcn_directory_t *directory )
{
    return APR_SUCCESS;
}


apr_status_t
lcn_ram_file_create ( lcn_ram_file_t **ram_file,
                      apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
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


static apr_status_t
lcn_ram_directory_list( const lcn_directory_t *directory,
                        lcn_list_t **file_list,
                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    struct ram_directory *ram_dir = (struct ram_directory*) directory;

    do
    {
        apr_hash_index_t *hi;

        LCNCE( lcn_list_create( file_list, 10, pool ));

        for( hi = apr_hash_first( pool, ram_dir->file_map); hi; hi = apr_hash_next( hi ))
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


static apr_status_t
lcn_ram_directory_create_file ( lcn_directory_t *directory,
                                lcn_ostream_t **new_os,
                                const char *file_name,
                                apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    struct ram_directory *ram_dir = (struct ram_directory*) directory;

    do
    {
        lcn_ram_file_t *file = (lcn_ram_file_t *) apr_hash_get( ram_dir->file_map, file_name, strlen(file_name) );

        if ( NULL == file )
        {
            char* fname_dup;

            LCNCE( lcn_ram_file_create( &file, directory->pool ) );

            /**
             * WARNING: it may be dangerous to let file own its name. We assume
             * here lifetime of file name == lifetime of file
             */
            LCNPV( fname_dup = apr_pstrdup( file->pool, file_name ), APR_ENOMEM );
            apr_hash_set( ram_dir->file_map, fname_dup, strlen(fname_dup), file );
        }

        LCNCE( lcn_ram_ostream_create( new_os, file, pool ) );
    }
    while(0);

    return s;
}


static apr_status_t
lcn_ram_directory_rename_file( lcn_directory_t *directory,
                               const char *from,
                               const char *to )
{
    apr_status_t s = APR_SUCCESS;
    struct ram_directory *ram_dir = (struct ram_directory*) directory;

    do
    {
        lcn_ram_file_t *file;

        LCNPV( file = (lcn_ram_file_t *) apr_hash_get( ram_dir->file_map, from, strlen(from) ),
               LCN_ERR_RAM_FILE_NOT_FOUND );

        apr_hash_set( ram_dir->file_map, to, strlen(to), file );
        apr_hash_set( ram_dir->file_map, from, strlen(from), NULL );
    }
    while(0);

    return s;
}


static apr_status_t
lcn_ram_directory_open_input( lcn_directory_t *directory,
                              lcn_index_input_t **new_in,
                              const char *file_name,
                              apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    struct ram_directory *ram_dir = (struct ram_directory*) directory;

    do
    {
        lcn_ram_file_t *file = (lcn_ram_file_t *) apr_hash_get( ram_dir->file_map, file_name, strlen(file_name) );

        if ( 0 == strcmp( "segments", file_name ) && NULL == file )
        {
            return LCN_ERR_RAM_FILE_NOT_FOUND;
        }

        LCNPV( file, LCN_ERR_RAM_FILE_NOT_FOUND );
        LCNCM( lcn_ram_input_stream_create( new_in, file_name, file, pool ), file_name );
    }
    while(0);

    return s;
}


static apr_status_t
lcn_ram_directory_file_exists( const lcn_directory_t *directory,
                               const char *file_name,
                               lcn_bool_t *file_exists )
{
    struct ram_directory *ram_dir = (struct ram_directory*) directory;

    if ( NULL != ( (lcn_ram_file_t *) apr_hash_get( ram_dir->file_map,
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

static apr_status_t
lcn_ram_directory_delete_file( lcn_directory_t *directory,
                               const char *file_name )
{
    struct ram_directory *ram_dir = (struct ram_directory*) directory;
    lcn_ram_file_t *file = (lcn_ram_file_t *) apr_hash_get( ram_dir->file_map, file_name, strlen(file_name) );

    if ( NULL != file )
    {
        apr_hash_set( ram_dir->file_map, file_name, strlen(file_name), NULL );
        apr_pool_destroy( file->pool );
    }

    return APR_SUCCESS;
}


apr_status_t
lcn_ram_directory_create( lcn_directory_t **new_dir, apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    struct ram_directory *ram_dir;

    do
    {
        LCNPV( ram_dir = (struct ram_directory *) apr_pcalloc( pool, sizeof(struct ram_directory) ), APR_ENOMEM );
        LCNCE( lcn_base_directory_init( (lcn_directory_t*)ram_dir, pool ));

        *new_dir = (lcn_directory_t*) ram_dir;

        ram_dir->file_map = apr_hash_make((*new_dir)->pool );

        (*new_dir)->name         = NAME;
        (*new_dir)->open_input   = lcn_ram_directory_open_input;
        (*new_dir)->_create_file = lcn_ram_directory_create_file;
        (*new_dir)->_delete_file = lcn_ram_directory_delete_file;
        (*new_dir)->_file_exists = lcn_ram_directory_file_exists;
        (*new_dir)->_rename_file = lcn_ram_directory_rename_file;
        (*new_dir)->_list        = lcn_ram_directory_list;
        (*new_dir)->_remove      = lcn_ram_directory_remove;
    }
    while(0);

    return s;
}
