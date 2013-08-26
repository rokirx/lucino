#include <lcn_util.h>
#include <lcn_store.h>

#include "index_input.h"

apr_off_t
lcn_index_output_get_file_pointer( lcn_index_output_t *io )
{
    return io->_get_file_pointer( io );
}

apr_off_t
lcn_index_output_get_file_pointer_impl( lcn_index_output_t *io )
{
    return io->buffer_start + io->buffer_position;
}

apr_status_t
lcn_index_output_close( lcn_index_output_t *io )
{
    return io->_close( io );
}

apr_status_t
lcn_index_output_close_impl( lcn_index_output_t *ostream )
{
    apr_status_t s = APR_SUCCESS;
    LCNCR( lcn_index_output_flush( ostream ) );
    ostream->isOpen = FALSE;
    return s;
}

apr_status_t
lcn_index_output_seek( lcn_index_output_t *ostream, apr_off_t pos )
{
    return ostream->_seek( ostream, pos );
}


apr_status_t
lcn_index_output_write_bytes( lcn_index_output_t *io,
                              const char *b,
                              unsigned int length )
{
    return io->_write_bytes( io, b, length );
}

apr_status_t
lcn_index_output_write_bytes_impl( lcn_index_output_t *io,
                                   const char *b,
                                   unsigned int length )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;

    if ( NULL != b )
    {
        for (i = 0; i < length; i++)
        {
            LCNCE( io->_write_byte( io, (unsigned char) b[i] ) );
        }
    }
    else
    {
        for (i = 0; i < length; i++)
        {
            LCNCE( io->_write_byte( io, (unsigned char) 0 ) );
        }
    }

    return s;
}

/**
 * Convenience method for function pointer _write_byte
 */
apr_status_t
lcn_index_output_write_byte ( lcn_index_output_t *os, unsigned char b)
{
    return os->_write_byte( os, b );
}

/**
 * Writes a single byte.
 *
 * The byte is just written to a buffer which is flushed if
 * full.
 */
apr_status_t
lcn_index_output_write_byte_impl ( lcn_index_output_t *os, unsigned char b)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( os->buffer_position >= LCN_STREAM_BUFFER_SIZE )
        {
            LCNCE( lcn_index_output_flush( os ) );
        }

        os->buffer[ os->buffer_position++ ] = (char) b;
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_output_write_int ( lcn_index_output_t *io, int n )
{
    apr_status_t s = APR_SUCCESS;

    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) n >> 24)) ) );
    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) n >> 16)) ) );
    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) n >>  8)) ) );
    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) n )) ) );

    return s;
}

/**
 * Writes the lower two bytes of an integer into the
 * output stream
 */
apr_status_t
lcn_index_output_write_int16( lcn_index_output_t *io, unsigned int i)
{
    apr_status_t s = APR_SUCCESS;

    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) i >>  8)) ) );
    LCNCR( io->_write_byte( io, (char) ((unsigned char) ((unsigned int) i )) ) );

    return s;
}

apr_status_t
lcn_index_output_write_bitvector( lcn_index_output_t *os, lcn_bitvector_t *bitvector )
{
    apr_status_t s;

    LCNCR( lcn_index_output_write_vint( os, (int) lcn_bitvector_size( bitvector ) )   );

    if ( NULL == lcn_bitvector_bits( bitvector ) )
    {
        LCNCR( lcn_bitvector_alloc_bits( bitvector ));
    }

    LCNCR( lcn_index_output_write_bytes( os,
                                         lcn_bitvector_bits( bitvector ),
                                         (size_t) ((lcn_bitvector_size( bitvector )>>3) + 1) ) );

    return s;
}

apr_status_t
lcn_index_output_write_vint( lcn_index_output_t *io, unsigned int i)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        while ( (i & ~0x7F) != 0)
        {
            LCNCE( io->_write_byte( io, (unsigned char)(i|0x80) ));
            i >>= 7;
        }

        if ( s )
        {
            break;
        }

        LCNCE( io->_write_byte( io, (unsigned char) i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_output_write_vlong( lcn_index_output_t *io, apr_uint64_t i)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        while( i & ~((apr_uint64_t) 0x7F)  )
        {
            LCNCE( io->_write_byte( io, (unsigned char)( (i & ((apr_uint64_t) 0x7f))) | 0x80 ) );
            i >>= 7;
        }

        if ( s )
        {
            break;
        }

        LCNCE( io->_write_byte( io, (unsigned char) i ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_output_write_long( lcn_index_output_t *os, apr_uint64_t i )
{
    apr_status_t s;

    LCNCR( lcn_index_output_write_int(os, (int) ((i >> 32 ) & 0xffffffff ) ) );
    LCNCR( lcn_index_output_write_int(os, (int) (i & 0xffffffff)));

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
lcn_index_output_write_chars ( lcn_index_output_t *io,
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
            LCNCE( io->_write_byte( io, (unsigned char) code ) );
        }
        else if ( ((code >= 0x80) && (code <= 0x7FF)) || code == 0)
        {
            LCNCE( io->_write_byte( io, (char) ((unsigned char) (0xC0 | (code >> 6))) ) );
            LCNCE( io->_write_byte( io, (char) ((unsigned char) (0x80 | (code & 0x3F))) ) );
        }
        else
        {
            LCNCE( io->_write_byte( io, (char) ((unsigned char) (0xE0 | (code >> 12))) ) );
            LCNCE( io->_write_byte( io, (char) ((unsigned char) ((code >> 6) & 0x3F)) ) );
            LCNCE( io->_write_byte( io, (char) ((unsigned char) (0x80 | (code & 0x3F))) ) );
        }
    }

    return s;
}

/**
 * Writes a string.
 */
apr_status_t
lcn_index_output_write_string( lcn_index_output_t *ostream,
                          const char *str )
{
    apr_status_t s;
    unsigned int length = strlen( str );

    LCNCR( lcn_index_output_write_vint( ostream, (unsigned int) length ) );
    LCNCR( lcn_index_output_write_chars( ostream, str, 0, length ) );

    return s;
}

apr_status_t
lcn_index_output_flush ( lcn_index_output_t *ostream )
{
    apr_status_t s;

    LCNCR( ostream->_flush_buffer( ostream,
                                   ostream->buffer,
                                   (unsigned int) ostream->buffer_position ) );
    ostream->buffer_start += ostream->buffer_position;
    ostream->buffer_position = 0;

    return s;
}

apr_status_t
lcn_index_output_init_struct ( lcn_index_output_t *new_os, apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( new_os->buffer = (char*) apr_palloc( pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE), APR_ENOMEM );

        new_os->pool = pool;
        new_os->buffer_length = LCN_STREAM_BUFFER_SIZE;
        new_os->buffer_position = 0;
        new_os->buffer_start = 0;
    }
    while(0);

    return s;
}
