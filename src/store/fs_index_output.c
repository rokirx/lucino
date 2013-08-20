#include <lcn_store.h>

#include "index_output.h"

static apr_status_t
lcn_fs_ostream_flush_buffer ( lcn_index_output_t *os, char *buf, size_t len )
{
    apr_status_t s;
    apr_size_t l = len;

    LCNCR( apr_file_write( os->_apr_file, buf, &l ) );
    
    return s;
}

/**
 * Sets current position in this file, where the next write will occur.
 */
static apr_status_t
lcn_fs_ostream_seek ( lcn_index_output_t *ostream, apr_off_t pos )
{
    apr_status_t s;
    apr_off_t p = pos;

    LCNCR( lcn_index_output_flush( ostream ) );
    ostream->buffer_start = pos;
    LCNCR( apr_file_seek( ostream->_apr_file, APR_SET, &p ) );

    return s;
}

/**
 * The number of bytes in the file.
 */
static apr_status_t
lcn_fs_ostream_length ( lcn_index_output_t *os, apr_off_t *len )
{
    apr_status_t s;
    apr_finfo_t finfo;

    LCNCR( apr_stat( &finfo, os->name, APR_FINFO_SIZE, os->pool ) );
    *len = finfo.size;

    return s;
}

apr_status_t
lcn_fs_ostream_create ( lcn_index_output_t **new_os,
                        const char *file_name,
                        apr_pool_t *pool )
{
    
    apr_status_t s;

    do
    {
        LCNPV( *new_os = (lcn_index_output_t*) apr_pcalloc( pool, sizeof(lcn_index_output_t)), APR_ENOMEM );
        LCNCE( lcn_index_output_init_struct( *new_os, pool ) );
        LCNPV( (*new_os)->name = apr_pstrdup( (*new_os)->pool, file_name ), APR_ENOMEM );
        LCNCE( apr_file_open( &((*new_os)->_apr_file),
                              file_name,
                              APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE,
                              APR_OS_DEFAULT,
                              (*new_os)->pool ) );
        
        (*new_os)->isOpen        = TRUE;
        (*new_os)->_length       = lcn_fs_ostream_length;
        (*new_os)->_seek         = lcn_fs_ostream_seek;
        (*new_os)->_flush_buffer = lcn_fs_ostream_flush_buffer;
    }
    while(0);

    return s;
}
