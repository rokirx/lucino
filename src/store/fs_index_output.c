#include <lcn_store.h>

#include "index_output.h"

typedef struct lcn_fs_index_output_t
{
    lcn_index_output_t io;
    
    apr_file_t *_apr_file;
    
} lcn_fs_index_output_t;

static apr_status_t
close( lcn_index_output_t *io ) 
{
    apr_status_t s;
    lcn_fs_index_output_t *fio = (lcn_fs_index_output_t*) io;   
    
    LCNCR( lcn_index_output_flush( io ) );
    LCNCR( apr_file_close( fio->_apr_file ) );
    fio->_apr_file = NULL;
    fio-> io.isOpen = FALSE;

    return s;
}

static apr_status_t
lcn_fs_ostream_flush_buffer ( lcn_index_output_t *os, char *buf, size_t len )
{
    apr_status_t s;
    apr_size_t l = len;
    
    LCNCR( apr_file_write( ( ( lcn_fs_index_output_t* ) os )->_apr_file, buf, &l ) );
    
    return s;
}

/**
 * Sets current position in this file, where the next write will occur.
 */
static apr_status_t
lcn_fs_ostream_seek ( lcn_index_output_t *io, apr_off_t pos )
{
    apr_status_t s;
    apr_off_t p = pos;

    LCNCR( lcn_index_output_flush( io ) );
    io->buffer_start = pos;
    LCNCR( apr_file_seek( ( ( lcn_fs_index_output_t*) io)->_apr_file, APR_SET, &p ) );

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
        lcn_fs_index_output_t *fio;
        
        LCNPV( fio = lcn_object_create( lcn_fs_index_output_t, pool ), APR_ENOMEM );
        LCNCE( lcn_index_output_init_struct( &fio->io, pool ) );
        LCNPV( fio->io.name = apr_pstrdup( fio->io.pool, file_name ), APR_ENOMEM );
        LCNCE( apr_file_open( &(fio->_apr_file),
                              file_name,
                              APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE,
                              APR_OS_DEFAULT,
                              fio->io.pool ) );
        
        fio->io.isOpen            = TRUE;
        fio->io._length           = lcn_fs_ostream_length;
        fio->io._seek             = lcn_fs_ostream_seek;
        fio->io._flush_buffer     = lcn_fs_ostream_flush_buffer;
        fio->io._close            = close;
        fio->io._write_byte       = lcn_index_output_write_byte_impl;
        fio->io._write_bytes      = lcn_index_output_write_bytes_impl;
        fio->io._get_file_pointer = lcn_index_output_get_file_pointer_impl;
        
        *new_os = (lcn_index_output_t*) fio;
        (*new_os)->type = "fs";
    }
    while(0);

    return s;
}
