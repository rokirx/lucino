#ifndef CRC32_H
#define	CRC32_H

typedef struct _lcn_crc32_t {
    
    unsigned int crc;
      
} lcn_crc32_t; 

apr_status_t
lcn_crc32_create( lcn_crc32_t** crc, apr_pool_t* pool );

void
lcn_crc32_update( lcn_crc32_t* crc, const void *buf, apr_size_t len);

void
lcn_crc32_reset( lcn_crc32_t* crc );

#endif	/* CRC32_H */

