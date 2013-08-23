#include <lucene.h>
#include <lcn_store.h>

#include "index_input.h"

typedef struct lcn_ram_index_output_t
{
    lcn_index_output_t io;
        
    lcn_ram_file_t *_file;  /* RAM lcn_index_output_t          */

    apr_off_t pointer;       /* RAM lcn_index_output_t position */
    
} lcn_ram_index_output_t;

static apr_status_t
lcn_ram_index_output_flush_buffer ( lcn_index_output_t *os, char *buf, size_t len )
{
    apr_status_t s;
    lcn_ram_index_output_t* rio = (lcn_ram_index_output_t*) os;
    char *new_buffer = NULL;
    char *buffer;
    size_t buffer_number = (size_t) (rio->pointer / LCN_STREAM_BUFFER_SIZE);
    off_t buffer_offset = rio->pointer % LCN_STREAM_BUFFER_SIZE;
    size_t bytes_in_buffer = (size_t) (LCN_STREAM_BUFFER_SIZE - buffer_offset);
    size_t bytes_to_copy   = bytes_in_buffer >= len ? len : bytes_in_buffer;
    lcn_list_t *buf_list = rio->_file->buffers;

    /* TODO try to clean up the code by moving allocation into the ram file */

    if ( ((size_t) buffer_number) == lcn_list_size( buf_list ) )
    {
        if ( NULL == (new_buffer = (char*) apr_pcalloc( rio->_file->pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE )))
        {
            return APR_ENOMEM;
        }

        if (( s = lcn_list_add( buf_list, new_buffer )))
        {
            return s;
        }
    }

    buffer = lcn_list_get( buf_list, buffer_number );
    memcpy( buffer + buffer_offset, buf, bytes_to_copy );

    if ( bytes_to_copy < len ) /* not all in one buffer */
    {
        size_t src_offset = bytes_to_copy;
        bytes_to_copy = len - bytes_to_copy; /* remaining bytes */
        buffer_number++;

        if ( buffer_number == lcn_list_size( buf_list ) )
        {
            if ( NULL == (new_buffer = (char*) apr_pcalloc( rio->_file->pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE )))
            {
                return APR_ENOMEM;
            }

            lcn_list_add( buf_list, new_buffer );
        }

        buffer = lcn_list_get( buf_list, buffer_number );
        memcpy( buffer, buf + src_offset, bytes_to_copy );
    }

    rio->pointer += len;

    if ( rio->pointer > rio->_file->length )
    {
        rio->_file->length = rio->pointer;
    }

    /* TODO set last modified in RAMFile
       file.lastModified = System.currentTimeMillis();
    */
    return APR_SUCCESS;
}

static apr_status_t
lcn_ram_index_output_seek ( lcn_index_output_t *os, apr_off_t pos )
{
    apr_status_t s;
    lcn_ram_index_output_t* rio = ( lcn_ram_index_output_t* ) os;

    LCNCR( lcn_index_output_flush( os ) );
    os->buffer_start = pos;
    rio->pointer = pos;

    return s;
}

static apr_status_t
lcn_ram_index_output_length ( lcn_index_output_t *os, apr_off_t *len )
{
    lcn_ram_index_output_t* rio = ( lcn_ram_index_output_t* ) os;
 
    *len = rio->_file->length;
    return APR_SUCCESS;
}

apr_off_t
lcn_ram_file_get_length( lcn_ram_file_t *ram_file )
{
    return ram_file->length;
}

apr_status_t
lcn_ram_index_output_write_to ( lcn_index_output_t *ram_ostream,
                                lcn_index_output_t *ostream )
{
    apr_status_t s;
    LCNCR( lcn_index_output_flush( ram_ostream ) );
    LCNCR( lcn_ram_file_copy_to_ostream( ( ( lcn_ram_index_output_t* ) ram_ostream )->_file, ostream ));
    return s;
}

apr_status_t
lcn_ram_index_output_reset( lcn_index_output_t *ostream )
{
    lcn_ram_index_output_t* rio = ( lcn_ram_index_output_t* ) ostream;
    
    ostream->buffer_start = 0;
    rio->pointer = 0;
    rio->_file->length = 0;

    return APR_SUCCESS;
}

apr_status_t
lcn_ram_file_copy_to_ostream ( lcn_ram_file_t *file,
                               lcn_index_output_t *ostream )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    char * buf;
    unsigned int buf_count = file->length / LCN_STREAM_BUFFER_SIZE;

    do
    {
        for ( i = 0; i < buf_count; i++ )
        {
            buf = lcn_list_get( file->buffers, i );
            LCNCE( lcn_index_output_write_bytes( ostream, buf, LCN_STREAM_BUFFER_SIZE ) );
        }

        if ( s )
        {
            break;
        }

        buf = lcn_list_get( file->buffers, i );
        i = file->length % LCN_STREAM_BUFFER_SIZE;
        LCNCE( lcn_index_output_write_bytes( ostream, buf, i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_ram_index_output_create ( lcn_index_output_t **new_os,
                              lcn_ram_file_t *file,
                              apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_ram_index_output_t *rio;
        
        LCNPV( rio = lcn_object_create( lcn_ram_index_output_t, pool ), APR_ENOMEM );
        LCNCE( lcn_index_output_init_struct( &rio->io, pool ) );

        rio->io.isOpen            = TRUE;
        rio->io._length           = lcn_ram_index_output_length;
        rio->io._seek             = lcn_ram_index_output_seek;
        rio->io._flush_buffer     = lcn_ram_index_output_flush_buffer;
        rio->io._write_byte       = lcn_index_output_write_byte_impl;
        rio->io._write_bytes      = lcn_index_output_write_bytes_impl;
        rio->io._close            = lcn_index_output_close_impl;
        rio->io._get_file_pointer = lcn_index_output_get_file_pointer_impl;
        
        rio->pointer = 0;
        rio->_file = file;
        
        *new_os = (lcn_index_output_t*) rio;
        (*new_os)->type = "ram";
    }
    while(0);
    
    return s;
}
