#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include "lucene.h"
#include "ostream.h"

#define STREAM_BUFFER_SIZE (1096)
#ifdef __cplusplus

#define BEGIN_C_DECLS extern "C" {
#define END_C_DECLS   }

#else

#define BEGIN_C_DECLS
#define END_C_DECLS

#endif /* __cplusplus */

BEGIN_C_DECLS

LUCENE_EXTERN unsigned int
LUCENE_API(lcn_istream_size) ( lcn_istream_t *input_stream );

/**
 * Creates an input stream which shares the base
 * file descriptor whith the original input stream.
 *
 * @param self The base InputStream
 * @param clone_is The cloned InputStream
 */
LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_istream_clone) ( lcn_istream_t *input_stream,
                                lcn_istream_t **clone,
                                apr_pool_t *pool );

/**
 * reads len bytes from InputStream into bytes starting at
 * offset
 *
 * Sets *len to the number of bytes read.
 */
apr_status_t
lcn_istream_read_bytes( lcn_istream_t *input_stream,
                        char *bytes,
                        apr_off_t offset,
                        unsigned int *len);

/**
 * Reads length characters into buf starting at position start.
 *
 * Returns APR_SUCCESS on success, error code on falure.
 *
 * buf must be at least of size (length + 1). 
 */
apr_status_t
lcn_istream_read_chars( lcn_istream_t *,
                        char *buf,
                        apr_off_t start,
                        unsigned int length);

apr_off_t
lcn_istream_file_pointer( lcn_istream_t *input_stream );

/**
 * Reads a byte (0<=b<=255) from InputStream. Returns -1 on error.
 */
apr_status_t
lcn_istream_read_byte( lcn_istream_t *input_stream,
                       unsigned char *result );

apr_status_t
lcn_istream_read_bitvector( lcn_istream_t *in, lcn_bitvector_t **bitvector, apr_pool_t *pool );

apr_status_t
lcn_istream_read_int( lcn_istream_t *input_stream,
                      int *result );

apr_status_t
lcn_istream_read_long( lcn_istream_t *input_stream,
                       apr_int64_t *result );

apr_status_t
lcn_istream_read_ulong( lcn_istream_t* input_stream,
                        apr_uint64_t* result );

apr_status_t
lcn_istream_read_vint( lcn_istream_t *input_stream,
                       unsigned int *result );

apr_status_t
lcn_istream_read_vlong( lcn_istream_t *istream,
                        apr_uint64_t *result );


/**
 * Reads a string into the buffer str.
 *
 * If *str is NULL, a new buffer is allocated from the given apr_pool_t and the
 * length of the new string is stored in *len.
 *
 * If str is not NULL, then *len should contain the size of buffer, the pool
 * may be NULL then. The length of actual string is then stored in *len. If
 * the buffer is not big enough, LUCENE_EBUFFER_TOO_SMALL is returned, the
 * minimum length of the buffer to store the string is stored in *len.
 */
apr_status_t
lcn_istream_read_string ( lcn_istream_t *input_stream,
                          char **str,
                          unsigned int *len,
                          apr_pool_t *pool );

apr_status_t
lcn_istream_seek( lcn_istream_t *input_stream,
                  apr_off_t offset );


/**
 * Closes the stream if the stream is open.
 */
LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_istream_close) ( lcn_istream_t *input_stream );

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_istream_create) ( lcn_istream_t **new_in,
                                 const char *file_name,
                                 apr_pool_t *pool );

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_ram_input_stream_create) ( lcn_istream_t **new_in,
                                          lcn_ram_file_t *ram_file,
                                          apr_pool_t *pool );

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_cs_input_stream_create) ( lcn_istream_t **new_is,
                                         lcn_istream_t *base,
                                         apr_off_t file_offset,
                                         apr_off_t length,
                                         apr_pool_t *pool);

END_C_DECLS

#endif /* INPUT_STREAM_H */

