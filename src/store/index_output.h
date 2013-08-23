#ifndef LCN_OSTREAM_H
#define LCN_OSTREAM_H

#include <lucene.h>
#include <lcn_bitvector.h>

#include "index_input.h"

struct lcn_ram_file_t {

    apr_pool_t *pool;

    /**
     * Internal list of buffers for output
     */
    lcn_list_t *buffers;

    /**
     * Size of the file
     */
    apr_off_t length;

    /**
     * last mod time of the file
     */
    unsigned int last_modified;

};

struct lcn_index_output_t  {

    apr_pool_t *pool;

    char *name;

    bool isOpen;

    char *buffer;
    
    /**
     * Debug
     */
    char *type;

    /**
     * file pointer = buffer_start + buffer_position
     */
    apr_off_t  buffer_start;      /* position in file of buffer */
    
    apr_off_t  buffer_position;   /* next byte to read          */
    
    unsigned int buffer_length;     /* end of valid bytes         */

    apr_status_t (*_length) ( lcn_index_output_t *ostream, apr_off_t *off);

    /**
     * Returns LUCENE_OK on success, LUCENE_IOERR on failure
     */
    apr_status_t (*_seek) ( lcn_index_output_t *ostream, apr_off_t off);

    /**
     * An abstract method, to be overriden by concrete classes.
     */
    apr_status_t (*_flush_buffer) ( lcn_index_output_t *ostream,
                                    char *buffer,
                                    size_t buf_size );
    
    /**
     * Lucene 5.0
     */
    
    /** Writes a single byte.
    * <p>
    * The most primitive data type is an eight-bit byte. Files are 
    * accessed as sequences of bytes. All other data types are defined 
    * as sequences of bytes, so file formats are byte-order independent.
    * 
    * @see IndexInput#readByte()
    */
    apr_status_t (*_write_byte) ( lcn_index_output_t *io,
                                  unsigned char b );

   /** Writes an array of bytes.
    * @param b the bytes to write
    * @param offset the offset in the byte array
    * @param length the number of bytes to write
    * @see DataInput#readBytes(byte[],int,int)
    */
    apr_status_t (*_write_bytes) ( lcn_index_output_t *io,
                                   const char *b,
                                   unsigned int length );
    
   /**
     * Closes this stream and releases any system resources associated
     * with it. If the stream is already closed then invoking this
     * method has no effect.
     */
    apr_status_t (*_close) ( lcn_index_output_t *io );
    
    /** Closes this stream to further operations. */
    apr_off_t (*_get_file_pointer) ( lcn_index_output_t *io );
};

#endif /* OSTREAM_H */
