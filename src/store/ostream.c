#include "lcn_util.h"
#include "lcn_store.h"

#include "index_input.h"


apr_off_t
lcn_ram_file_get_length( lcn_ram_file_t *ram_file )
{
    return ram_file->length;
}

apr_status_t
lcn_ram_ostream_write_to ( lcn_ostream_t *ram_ostream,
                           lcn_ostream_t *ostream )
{
    apr_status_t s;
    LCNCR( lcn_ostream_flush( ram_ostream ) );
    LCNCR( lcn_ram_file_copy_to_ostream( ram_ostream->_file, ostream ));
    return s;
}

apr_status_t
lcn_ram_ostream_reset( lcn_ostream_t *ostream )
{
    ostream->buffer_start = 0;
    ostream->pointer = 0;
    ostream->_file->length = 0;

    return APR_SUCCESS;
}

apr_off_t
lcn_ostream_get_file_pointer( lcn_ostream_t *ostream )
{
    return ostream->buffer_start + ostream->buffer_position;
}

apr_status_t
lcn_ostream_close( lcn_ostream_t *ostream )
{
    apr_status_t s;

    LCNCR( lcn_ostream_flush( ostream ) );

    if( NULL != ostream->_apr_file )
    {
        LCNCR( apr_file_close( ostream->_apr_file ) );
        ostream->_apr_file = NULL;
    }

    ostream->isOpen = FALSE;

    return s;
}

apr_status_t
lcn_ostream_seek( lcn_ostream_t *ostream, apr_off_t pos )
{
    apr_status_t s;
    LCNCR( ostream->_seek( ostream, pos ) );
    return s;
}

apr_status_t
lcn_ostream_write_bytes( lcn_ostream_t *ostream,
                         const char *b,
                         unsigned int length )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;

    if ( NULL != b )
    {
        for (i = 0; i < length; i++)
        {
            LCNCE( lcn_ostream_write_byte( ostream, (unsigned char) b[i] ) );
        }
    }
    else
    {
        for (i = 0; i < length; i++)
        {
            LCNCE( lcn_ostream_write_byte( ostream, (unsigned char) 0 ) );
        }
    }

    return s;
}

/**
 * Writes a single byte.
 *
 * The byte is just written to a buffer which is flushed if
 * full.
 */
apr_status_t
lcn_ostream_write_byte ( lcn_ostream_t *os, unsigned char b)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( os->buffer_position >= LCN_STREAM_BUFFER_SIZE )
        {
            LCNCE( lcn_ostream_flush( os ) );
        }

        os->buffer[ os->buffer_position++ ] = (char) b;
    }
    while(0);

    return s;
}

apr_status_t
lcn_ostream_write_int ( lcn_ostream_t *os, int n )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( os->buffer_position + 3 >= LCN_STREAM_BUFFER_SIZE )
        {
            LCNCE( lcn_ostream_flush( os ) );
        }

        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) n >> 24));
        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) n >> 16));
        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) n >>  8));
        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) n ));
    }
    while(0);

    return s;
}

/**
 * Writes the lower two bytes of an integer into the
 * output stream
 */
apr_status_t
lcn_ostream_write_int16( lcn_ostream_t *os, unsigned int i)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( os->buffer_position + 1 >= LCN_STREAM_BUFFER_SIZE )
        {
            LCNCE( lcn_ostream_flush( os ) );
        }

        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) i >>  8));
        os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((unsigned int) i ));
    }
    while(0);

    return s;
}


apr_status_t
lcn_ostream_write_bitvector( lcn_ostream_t *os, lcn_bitvector_t *bitvector )
{
    apr_status_t s;

    LCNCR( lcn_ostream_write_vint( os, (int) lcn_bitvector_size( bitvector ) )   );

    if ( NULL == lcn_bitvector_bits( bitvector ) )
    {
        LCNCR( lcn_bitvector_alloc_bits( bitvector ));
    }

    LCNCR( lcn_ostream_write_bytes( os,
                                    lcn_bitvector_bits( bitvector ),
                                    (size_t) ((lcn_bitvector_size( bitvector )>>3) + 1) ) );

    return s;
}

apr_status_t
lcn_ostream_write_vint( lcn_ostream_t *os, unsigned int i)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        while ( (i & ~0x7F) != 0)
        {
            LCNCE( lcn_ostream_write_byte( os, (unsigned char)(i|0x80) ));
            i >>= 7;
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_ostream_write_byte( os, (unsigned char) i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_ostream_write_vlong( lcn_ostream_t *os, apr_uint64_t i)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        while( i & ~((apr_uint64_t) 0x7F)  )
        {
            LCNCE( lcn_ostream_write_byte( os, (unsigned char)( (i & ((apr_uint64_t) 0x7f))) | 0x80 ) );
            i >>= 7;
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_ostream_write_byte( os, (unsigned char) i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_ostream_write_long( lcn_ostream_t *os, apr_uint64_t i )
{
    apr_status_t s;

    LCNCR( lcn_ostream_write_int(os, (int) ((i >> 32 ) & 0xffffffff ) ) );
    LCNCR( lcn_ostream_write_int(os, (int) (i & 0xffffffff)));

    return s;
}

/**
 * Writes a sequence of UTF-8 encoded characters from a string.
 *
 * @param s the source of the characters
 * @param start the first character in the sequence
 * @param length the number of characters in the sequence
 *
 * returns LUCENE_OK on success, LUCENE_IOERR on failure
 */
apr_status_t
lcn_ostream_write_chars ( lcn_ostream_t *os,
                          const char *str,
                          apr_off_t start,
                          unsigned int length )
{
    apr_status_t s = APR_SUCCESS;

    apr_off_t end = start + length;
    apr_off_t i;

    for ( i = start; i < end; i++)
    {
        unsigned int code = (unsigned int) (unsigned char) str[i];

        if ( code >= 0x01 && code <= 0x7F )
        {
            LCNCE( lcn_ostream_write_byte( os, (unsigned char) code ) );
        }
        else if ( ((code >= 0x80) && (code <= 0x7FF)) || code == 0)
        {
            if ( os->buffer_position + 1 >= LCN_STREAM_BUFFER_SIZE )
            {
                LCNCE( lcn_ostream_flush( os ) );
            }

            os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) (0xC0 | (code >> 6)));
            os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) (0x80 | (code & 0x3F)));
        }
        else
        {
            if ( os->buffer_position + 2 >= LCN_STREAM_BUFFER_SIZE )
            {
                LCNCE( lcn_ostream_flush( os ) );
            }

            os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) (0xE0 | (code >> 12)));
            os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) ((code >> 6) & 0x3F));
            os->buffer[ os->buffer_position++ ] = (char) ((unsigned char) (0x80 | (code & 0x3F)));
        }
    }

    return s;
}

/**
 * Writes a string.
 */
apr_status_t
lcn_ostream_write_string( lcn_ostream_t *ostream,
                          const char *str )
{
    apr_status_t s;
    unsigned int length = strlen( str );

    LCNCR( lcn_ostream_write_vint( ostream, (unsigned int) length ) );
    LCNCR( lcn_ostream_write_chars( ostream, str, 0, length ) );

    return s;
}

static apr_status_t
lcn_fs_ostream_flush_buffer ( lcn_ostream_t *os, char *buf, size_t len )
{
    apr_status_t s;
    apr_size_t l = len;

    LCNCR( apr_file_write( os->_apr_file, buf, &l ) );

    return s;
}

static apr_status_t
lcn_ram_ostream_flush_buffer ( lcn_ostream_t *os, char *buf, size_t len )
{
    apr_status_t s;
    char *new_buffer = NULL;
    char *buffer;
    size_t buffer_number = (size_t) (os->pointer / LCN_STREAM_BUFFER_SIZE);
    off_t buffer_offset = os->pointer % LCN_STREAM_BUFFER_SIZE;
    size_t bytes_in_buffer = (size_t) (LCN_STREAM_BUFFER_SIZE - buffer_offset);
    size_t bytes_to_copy   = bytes_in_buffer >= len ? len : bytes_in_buffer;
    lcn_list_t *buf_list = os->_file->buffers;

    /* TODO try to clean up the code by moving allocation into the ram file */

    if ( ((size_t) buffer_number) == lcn_list_size( buf_list ) )
    {
        if ( NULL == (new_buffer = (char*) apr_pcalloc( os->_file->pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE )))
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
            if ( NULL == (new_buffer = (char*) apr_pcalloc( os->_file->pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE )))
            {
                return APR_ENOMEM;
            }

            lcn_list_add( buf_list, new_buffer );
        }

        buffer = lcn_list_get( buf_list, buffer_number );
        memcpy( buffer, buf + src_offset, bytes_to_copy );
    }

    os->pointer += len;

    if ( os->pointer > os->_file->length )
    {
        os->_file->length = os->pointer;
    }

    /* TODO set last modified in RAMFile
       file.lastModified = System.currentTimeMillis();
    */
    return APR_SUCCESS;
}


apr_status_t
lcn_ostream_flush ( lcn_ostream_t *ostream )
{
    apr_status_t s;

    LCNCR( ostream->_flush_buffer( ostream,
                                   ostream->buffer,
                                   (unsigned int) ostream->buffer_position ) );
    ostream->buffer_start += ostream->buffer_position;
    ostream->buffer_position = 0;

    return s;
}


/**
 * Sets current position in this file, where the next write will occur.
 */
static apr_status_t
lcn_fs_ostream_seek ( lcn_ostream_t *ostream, apr_off_t pos )
{
    apr_status_t s;
    apr_off_t p = pos;

    LCNCR( lcn_ostream_flush( ostream ) );
    ostream->buffer_start = pos;
    LCNCR( apr_file_seek( ostream->_apr_file, APR_SET, &p ) );

    return s;
}

static apr_status_t
lcn_ram_ostream_seek ( lcn_ostream_t *os, apr_off_t pos )
{
    apr_status_t s;

    LCNCR( lcn_ostream_flush( os ) );
    os->buffer_start = pos;
    os->pointer = pos;

    return s;
}

/**
 * The number of bytes in the file.
 */
static apr_status_t
lcn_fs_ostream_length ( lcn_ostream_t *os, apr_off_t *len )
{
    apr_status_t s;
    apr_finfo_t finfo;

    LCNCR( apr_stat( &finfo, os->name, APR_FINFO_SIZE, os->pool ) );
    *len = finfo.size;

    return s;
}

static apr_status_t
lcn_ram_ostream_length ( lcn_ostream_t *os, apr_off_t *len )
{
    *len = os->_file->length;
    return APR_SUCCESS;
}

static apr_status_t
init_ostream_struct ( lcn_ostream_t **new_os, apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *new_os = (lcn_ostream_t*) apr_pcalloc( pool, sizeof(lcn_ostream_t)),
               APR_ENOMEM );

        LCNPV( (*new_os)->buffer = (char*) apr_palloc( pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE),
               APR_ENOMEM );

        (*new_os)->pool = pool;
        (*new_os)->buffer_length = LCN_STREAM_BUFFER_SIZE;
        (*new_os)->buffer_position = 0;
        (*new_os)->buffer_start = 0;
    }
    while(0);

    return s;
}


static apr_status_t
create_ostream_internal ( lcn_ostream_t **new_os,
                          const char *file_name,
                          apr_int32_t flag,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNCE( init_ostream_struct( new_os, pool ) );
        LCNPV( (*new_os)->name = apr_pstrdup( (*new_os)->pool, file_name ), APR_ENOMEM );
        LCNCE( apr_file_open( &((*new_os)->_apr_file),
                              file_name,
                              flag,
                              APR_OS_DEFAULT,
                              (*new_os)->pool ) );
        (*new_os)->isOpen = TRUE;

        (*new_os)->_length       = lcn_fs_ostream_length;
        (*new_os)->_seek         = lcn_fs_ostream_seek;
        (*new_os)->_flush_buffer = lcn_fs_ostream_flush_buffer;
    }
    while(0);

    return s;
}

apr_status_t
create_appending_ostream( lcn_ostream_t **new_os,
                          const char *file_name,
                          apr_pool_t *pool )
{
    apr_status_t s;
    apr_off_t len;

    LCNCR( create_ostream_internal( new_os, file_name, APR_FOPEN_APPEND | APR_FOPEN_WRITE, pool ) );
    LCNCR( (*new_os)->_length( *new_os, &len ) );
    LCNCR( (*new_os)->_seek( *new_os, len) );

    return s;
}


apr_status_t
lcn_ram_file_copy_to_ostream ( lcn_ram_file_t *file,
                               lcn_ostream_t *ostream )
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
            LCNCE( lcn_ostream_write_bytes( ostream, buf, LCN_STREAM_BUFFER_SIZE ) );
        }

        if ( s )
        {
            break;
        }

        buf = lcn_list_get( file->buffers, i );
        i = file->length % LCN_STREAM_BUFFER_SIZE;
        LCNCE( lcn_ostream_write_bytes( ostream, buf, i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_fs_ostream_create ( lcn_ostream_t **new_os,
                        const char *file_name,
                        apr_pool_t *pool )
{
    return create_ostream_internal( new_os, file_name,
                APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE,
                                          pool );
}

apr_status_t
lcn_ram_ostream_create ( lcn_ostream_t **new_os,
                         lcn_ram_file_t *file,
                         apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( init_ostream_struct( new_os, pool ) );

    (*new_os)->_length       = lcn_ram_ostream_length;
    (*new_os)->_seek         = lcn_ram_ostream_seek;
    (*new_os)->_flush_buffer = lcn_ram_ostream_flush_buffer;

    (*new_os)->pointer = 0;
    (*new_os)->_file = file;

    (*new_os)->isOpen = TRUE;

    return s;
}
