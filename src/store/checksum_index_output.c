#include <lucene.h>
#include <lcn_store.h>
#include <unistd.h>

#include "index_output.h"
#include "../util/crc32.h"
#include "directory.h"
#include "index_input.h"

typedef struct lcn_checksum_index_output_t
{
    lcn_index_output_t os;

    lcn_index_output_t *main;

    lcn_crc32_t *digest;
} lcn_checksum_index_output_t;

static apr_status_t
lcn_checksum_index_output_flush_buffer( lcn_index_output_t *io, char *c, size_t size )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;
    LCNCR( cio->main->_flush_buffer( cio->main, c, size ) );
    return s;
}

static apr_status_t
lcn_checksum_index_output_seek( lcn_index_output_t *io, apr_off_t buf_size )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;
    LCNCR( cio->main->_seek( cio->main, buf_size ) );
    return s;
}

static apr_status_t
lcn_checksum_index_output_length( lcn_index_output_t *io, apr_off_t *buf_size )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;
    LCNCR( cio->main->_length( cio->main, buf_size ) );
    return s;
}

static apr_status_t
lcn_checksum_index_output_close( lcn_index_output_t *io )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;
    LCNCR( cio->main->_close( cio->main ) );
    return s;
}

static apr_off_t
lcn_checksum_index_output_get_file_pointer( lcn_index_output_t *io )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;
    return cio->main->_get_file_pointer( cio->main );
}

static apr_status_t
lcn_checksum_index_output_write_byte ( lcn_index_output_t *io,
                                       unsigned char b )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) io;

    lcn_crc32_update( cio->digest, (const void*) &b, (unsigned int) 1 );
    return lcn_index_output_write_byte( cio->main, b );
}

static apr_status_t
lcn_checksum_index_output_write_bytes( lcn_index_output_t *os,
                                       const char *buf,
                                       unsigned int len )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) os;

    lcn_crc32_update( cio->digest, (const void*) buf, len );
    return lcn_index_output_write_bytes( cio->main, buf, (unsigned int) len );
}

apr_status_t
lcn_checksum_index_output_create( lcn_index_output_t **os,
                                  lcn_index_output_t *main,
                                  apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_checksum_index_output_t *cio;

        LCNPV( cio = lcn_object_create( lcn_checksum_index_output_t, pool ), APR_ENOMEM );
        LCNCE( lcn_index_output_init_struct( &cio->os, pool ) );
        LCNCE( lcn_crc32_create( &cio->digest, pool) );

        cio->main = main;

        cio->os.isOpen            = cio->main->isOpen;
        cio->os.name              = cio->main->name;

        cio->os._flush_buffer     = lcn_checksum_index_output_flush_buffer;
        cio->os._length           = lcn_checksum_index_output_length;
        cio->os._seek             = lcn_checksum_index_output_seek;

        cio->os._get_file_pointer = lcn_checksum_index_output_get_file_pointer;
        cio->os._close            = lcn_checksum_index_output_close;

        cio->os._write_byte       = lcn_checksum_index_output_write_byte;
        cio->os._write_bytes      = lcn_checksum_index_output_write_bytes;

        *os = (lcn_index_output_t*) cio;
    }
    while(0);

    return s;
}

unsigned int
lcn_checksum_index_output_get_checksum( lcn_index_output_t *os )
{
    return ( ( lcn_checksum_index_output_t* ) os )->digest->crc;
}

apr_status_t
lcn_checksum_index_output_finish_commit( lcn_index_output_t *os )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) os;
    unsigned int crc = cio->digest->crc;

    return lcn_index_output_write_long( cio->main, crc );
}

apr_status_t
lcn_checksum_index_output_write_string_string_hash( lcn_index_output_t *os,
                                                    apr_hash_t *hash )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = (lcn_checksum_index_output_t*) os;

    if ( NULL == hash )
    {
        LCNCR( lcn_index_output_write_int( cio, 0 ) );
    }
    else
    {
        unsigned int size = apr_hash_count( hash );
        lcn_index_output_write_int( cio, size );
#if 0
        TODO: implement

        for(final Map.Entry<String, String> entry: map.entrySet()) {
        writeString(entry.getKey());
        writeString(entry.getValue());
      }
#endif
    }

    return s;
}

#if 0
  TODO implement

  @Override
  public long length() throws IOException {
    return main.length();
  }
#endif
