#include <lucene.h>
#include <lcn_store.h>

#include "ostream.h"
#include "../util/crc32.h"
#include "directory.h"
#include "index_input.h"

struct _lcn_checksum_index_output_t 
{
    lcn_ostream_t os;
    
    lcn_ostream_t *main;
    
    lcn_crc32_t *digest;
};

apr_status_t
lcn_checksum_index_output_create( lcn_ostream_t **os,
                                  lcn_ostream_t *main,
                                  apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    
    do
    { 
        lcn_checksum_index_output_t *cio;
        
        LCNPV( cio = lcn_object_create( lcn_checksum_index_output_t, pool ), APR_ENOMEM );
        LCNCE( lcn_init_ostream_struct( &cio->os, pool ) );
        cio->main = main;
        LCNCE( lcn_crc32_create( &cio->digest, pool) ); 
        
        *os = (lcn_ostream_t*) cio;
    }
    while(0);
    
    return s;
}

apr_status_t
lcn_checksum_index_output_write_bytes( lcn_ostream_t *os,
                                       const char *buf,
                                       apr_size_t len )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) os;
    
    lcn_crc32_update( cio->digest, (const void*) buf, len );
    return lcn_ostream_write_bytes( cio->main, buf, (unsigned int) len );
}

unsigned int
lcn_checksum_index_output_get_checksum( lcn_ostream_t *os )
{
    return ( ( lcn_checksum_index_output_t* ) os )->digest->crc;
}

apr_status_t
lcn_checksum_index_output_finish_commit( lcn_ostream_t *os )
{
    lcn_checksum_index_output_t *cio = ( lcn_checksum_index_output_t* ) os;
    unsigned int crc = cio->digest->crc;
    
    return lcn_ostream_write_long( cio->main, crc );
}

/**
 * TODO: Nach index_output (ostream) verschieben 
 */
apr_status_t
lcn_checksum_index_output_write_string_string_hash( lcn_ostream_t *os, 
                                                    apr_hash_t *hash )
{
    apr_status_t s = APR_SUCCESS;
    lcn_checksum_index_output_t *cio = (lcn_checksum_index_output_t*) os;    
    
    if ( NULL == hash )
    {
        LCNCR( lcn_ostream_write_int( cio->main, 0 ) );
    }
    else
    {
        unsigned int size = apr_hash_count( hash ); 
        lcn_ostream_write_int( cio->main, size );
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
  
/**
 * TODO remove hack. Replace with function pointer
 */
apr_status_t
lcn_checksum_index_output_close( lcn_ostream_t* os )
{
    apr_status_t s = APR_SUCCESS;
    
    LCNCR( lcn_ostream_close( ( ( lcn_checksum_index_output_t* ) os)->main ) );
    
    return s;
}
