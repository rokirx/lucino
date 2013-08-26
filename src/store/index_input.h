#ifndef INDEX_INPUT_H
#define INDEX_INPUT_H

#include "lucene.h"
#include "index_output.h"

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
LUCENE_API(lcn_index_input_size) ( lcn_index_input_t *input_stream );

struct lcn_index_input_t {

    /* APR Pool */
    apr_pool_t *pool;

    /**
     * APR-File handle. Clones may not release the resource.
     */
    apr_file_t *_file_desc;

    /**
     * Name of file of this InputStream. Memory management
     * same as file handle.
     */
    char *name;

    lcn_bool_t is_open;
    lcn_bool_t is_clone;

    /**
     * Internal Buffer for this InputStream.
     */
    char *buffer;

    /**
     * Length of the file
     */
    apr_off_t size;

    /**
     * Position of the end of buffer in file
     */
    apr_off_t position;

    /**
     * Position in file of buffer
     */
    apr_off_t buffer_start;

    /**
     * Index of next byte to read in buffer.
     */
    apr_off_t buffer_position;

    apr_off_t pointer;

    /**
     * Length of the buffer ( end of valid bytes )
     */
    unsigned int buffer_length;

    /**
     * Reads a specified number of bytes into an array at the specified offset.
     *
     * @param b the array to read bytes into
     * @param offset the offset in the array to start storing bytes
     * @param len the number of bytes to read
     * @see OutputStream#writeBytes(byte[],int)
     *
     * returns status code if some problem occured, APR_SUCCESS on success
     */
    apr_status_t (*_read_internal) ( lcn_index_input_t *,
                                     char *dest,
                                     apr_off_t offset,
                                     unsigned int len);

    /**
     * concrete implementation of the clone method
     *
     * @param istream The base InputStream
     * @param clone_is The cloned InputStream
     */
    apr_status_t (*_clone) ( lcn_index_input_t *istream,
                             lcn_index_input_t **clone,
                             apr_pool_t *pool );

    apr_status_t (*_close)( lcn_index_input_t *istream );

    /* CSInputStream */
    lcn_index_input_t *base;
    apr_off_t file_offset;
};


/**
 * Creates an input stream which shares the base
 * file descriptor whith the original input stream.
 *
 * @param self The base InputStream
 * @param clone_is The cloned InputStream
 */
apr_status_t
lcn_index_input_clone( lcn_index_input_t *input_stream,
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
lcn_index_input_init( lcn_index_input_t *in,
                      const char *name,
                      apr_pool_t *pool );


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
lcn_index_input_read_bitvector( lcn_index_input_t *in,
                                lcn_bitvector_t **bitvector,
                                apr_pool_t *pool );

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
lcn_index_input_read_string ( lcn_index_input_t *input_stream,
                              char **str,
                              unsigned int *len,
                              apr_pool_t *pool );

apr_status_t
lcn_index_input_seek( lcn_index_input_t *input_stream,
                      apr_off_t offset );


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
lcn_cs_input_stream_create( lcn_index_input_t **new_is,
                            lcn_index_input_t *base,
                            apr_off_t file_offset,
                            apr_off_t length,
                            apr_pool_t *pool);

END_C_DECLS

#endif /* INDEX_INPUT_H */

