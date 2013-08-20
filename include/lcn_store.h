#ifndef LCN_STORE_H
#define LCN_STORE_H

#include "lucene.h"

#define LCN_STREAM_BUFFER_SIZE (1096)

BEGIN_C_DECLS

/**
 * @defgroup store Storing
 * @ingroup lucene
 * @{
 */

/**
 * @defgroup istream Input-Stream
 * @ingroup store
 * @{
 */

apr_status_t
lcn_index_output_init_struct ( lcn_index_output_t *new_os, apr_pool_t *pool );

unsigned int
lcn_index_input_size ( lcn_index_input_t *input_stream );

/**
 * Creates an input stream which shares the base
 * file descriptor whith the original input stream.
 *
 * @param self The base InputStream
 * @param clone_is The cloned InputStream
 */
apr_status_t
lcn_index_input_clone ( lcn_index_input_t *input_stream,
                    lcn_index_input_t **clone,
                    apr_pool_t *pool );

/**
 * reads len bytes from InputStream into bytes starting at
 * offset
 *
 * Sets *len to the number of bytes read.
 */
apr_status_t
lcn_index_input_read_bytes( lcn_index_input_t *input_stream,
                        char *bytes,
                        apr_off_t offset,
                        unsigned int *len);

apr_status_t
lcn_index_input_read_int16 ( lcn_index_input_t *istream,
                         int *result );

/**
 * Reads length characters into buf starting at position start.
 *
 * Returns APR_SUCCESS on success, error code on falure.
 *
 * buf must be at least of size (length + 1).
 */
apr_status_t
lcn_index_input_read_chars( lcn_index_input_t *,
                        char *buf,
                        apr_off_t start,
                        unsigned int length);

apr_off_t
lcn_index_input_file_pointer( lcn_index_input_t *input_stream );

/**
 * Reads a byte (0<=b<=255) from InputStream. Returns -1 on error.
 */
apr_status_t
lcn_index_input_read_byte( lcn_index_input_t *input_stream,
                       unsigned char *result );

apr_status_t
lcn_index_input_read_bitvector( lcn_index_input_t *in, lcn_bitvector_t **bitvector, apr_pool_t *pool );

apr_status_t
lcn_index_input_read_int( lcn_index_input_t *input_stream,
                      int *result );

apr_status_t
lcn_index_input_read_int( lcn_index_input_t *input_stream,
                      int *result );

apr_status_t
lcn_index_input_read_long( lcn_index_input_t *input_stream,
                       apr_int64_t *result );

apr_status_t
lcn_index_input_read_ulong( lcn_index_input_t* input_stream,
                        apr_uint64_t* result );

apr_status_t
lcn_index_input_read_vint( lcn_index_input_t *input_stream,
                       unsigned int *result );

apr_status_t
lcn_index_input_read_vsize( lcn_index_input_t *input_stream,
                        unsigned int *result );

apr_status_t
lcn_index_input_read_vlong( lcn_index_input_t *istream,
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
lcn_index_input_read_string( lcn_index_input_t *input_stream,
                             char **str,
                             unsigned int *len,
                             apr_pool_t *pool );

apr_status_t
lcn_index_input_seek( lcn_index_input_t *input_stream,
                  apr_off_t offset );

apr_status_t
lcn_index_input_name( lcn_index_input_t *input_stream,
                      char **name,
                      apr_pool_t *pool);


/**
 * Closes the stream if the stream is open.
 */
apr_status_t
lcn_index_input_close( lcn_index_input_t *input_stream );

apr_status_t
lcn_index_input_create( lcn_index_input_t **new_in,
                        const char *file_name,
                        apr_pool_t *pool );
apr_status_t
lcn_ram_input_stream_create( lcn_index_input_t **new_in,
                             const char *name,
                             lcn_ram_file_t *ram_file,
                             apr_pool_t *pool );

/**
 * @brief Creates an input stream getting bytes from buffer
 *
 * @param new_in  neuer input
 * @param buffer  data buffer
 * @param len     length of buffer
 * @param pool    APR pool
 */
apr_status_t
lcn_index_input_buf_stream_create ( lcn_index_input_t **new_in,
                                    const char *buffer,
                                    unsigned int len,
                                    apr_pool_t *pool );


apr_status_t
lcn_cs_input_stream_create( lcn_index_input_t **new_is,
                            lcn_index_input_t *base,
                            apr_off_t file_offset,
                            apr_off_t length,
                            apr_pool_t *pool);

/** @} */

/**
 * @defgroup ostream Output-Stream
 * @ingroup store
 * @{
 */

apr_off_t
lcn_index_output_get_file_pointer( lcn_index_output_t *ostream );


apr_status_t
lcn_index_output_seek( lcn_index_output_t *ostream, apr_off_t pos );


apr_status_t
lcn_index_output_write_chars( lcn_index_output_t *os,
                         const char *s,
                         apr_off_t start,
                         unsigned int length );

apr_status_t
lcn_index_output_write_vlong( lcn_index_output_t *ostream,
                         apr_uint64_t n );

/**
 * Writes a long as eight bytes.
 */
apr_status_t
lcn_index_output_write_long( lcn_index_output_t *ostream,
                        apr_uint64_t n );

/**
 * Writes an unsigned int as four bytes.
 */
apr_status_t
lcn_index_output_write_int16( lcn_index_output_t *ostream,
                         unsigned int n );


apr_status_t
lcn_index_output_write_byte( lcn_index_output_t *ostream,
                        unsigned char b );


/**
 * Writes an array of bytes.
 *
 * @param b the bytes to write
 * @param length the number of bytes to write
 *
 */
apr_status_t
lcn_index_output_write_bytes( lcn_index_output_t *ostream,
                         const char *buf,
                         unsigned int size );

/**
 * Forces any buffered output to be written. Resets the
 * buffer for further usage
 *
 */
apr_status_t
lcn_index_output_flush( lcn_index_output_t *ostream );

apr_status_t
lcn_ram_ostream_reset( lcn_index_output_t *ostream );

/**
 * Writes an int in a variable-length format.  Writes between one and
 * five bytes.  Smaller values take fewer bytes.  Negative numbers are not
 * supported.
 */
apr_status_t
lcn_index_output_write_vint( lcn_index_output_t *ostream,
                        unsigned int n );

apr_status_t
lcn_index_output_write_bitvector( lcn_index_output_t *os, lcn_bitvector_t *bitvector );


/**
 * Writes a string to the output
 */
apr_status_t
lcn_index_output_write_string( lcn_index_output_t *ostream,
                          const char *str );

/**
 * Flushes the buffers and cloes files of the output stream
 */
apr_status_t
lcn_index_output_close( lcn_index_output_t *ostream );

/**
 * Writes an int as four bytes. It must be possible
 * to write negative integers.
 */
apr_status_t
lcn_index_output_write_int ( lcn_index_output_t *ostream, int n );


/**
 * Opens an existing file for appending the output
 */
apr_status_t
lcn_appending_ostream_create( lcn_index_output_t **new_os,
                              const char *file_name );

/**
 * Creates a new file for writing the output
 */
apr_status_t
lcn_fs_ostream_create( lcn_index_output_t **new_os,
                       const char *file_name,
                       apr_pool_t *pool );

apr_status_t
lcn_ram_ostream_create( lcn_index_output_t **new_os,
                        lcn_ram_file_t *file,
                        apr_pool_t *pool );

apr_status_t
lcn_ram_file_create ( lcn_ram_file_t **ram_file,
                      apr_pool_t *pool );

apr_status_t
lcn_ram_file_copy_to_ostream ( lcn_ram_file_t *file,
                               lcn_index_output_t *out );

apr_status_t
lcn_ram_ostream_write_to ( lcn_index_output_t *ram_ostream,
                           lcn_index_output_t *ostream );

apr_status_t
lcn_checksum_index_output_create( lcn_index_output_t **os,
                                  lcn_index_output_t *main,
                                  apr_pool_t *pool );

apr_status_t
lcn_checksum_index_output_write_bytes( lcn_index_output_t *os,
                                       const char *buf,
                                       apr_size_t len );

unsigned int
lcn_checksum_index_output_get_checksum( lcn_index_output_t *os );

apr_status_t
lcn_checksum_index_output_finish_commit( lcn_index_output_t *os );

apr_status_t
lcn_checksum_index_output_write_string_string_hash( lcn_index_output_t *os, 
                                                    apr_hash_t *hash );
apr_status_t
lcn_checksum_index_output_close( lcn_index_output_t* os );

/** @} */
/** @} */

apr_off_t
lcn_ram_file_get_length ( lcn_ram_file_t *ram_file );

apr_status_t
lcn_file_exists( const char *file_name, lcn_bool_t *flag, apr_pool_t *pool );

void
lcn_ostream_copy( lcn_index_output_t *src, lcn_index_output_t *dst );

END_C_DECLS

#endif /* LCN_STORE_H */
