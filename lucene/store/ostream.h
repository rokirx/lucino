#ifndef LCN_OSTREAM_H
#define LCN_OSTREAM_H

#include "lucene.h"
#include "lcn_bitvector.h"


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




struct lcn_ostream_t  {

    apr_pool_t *pool;
    apr_file_t *_apr_file;

    lcn_ram_file_t * _file;  /* RAM lcn_ostream_t          */
    apr_off_t pointer;       /* RAM lcn_ostream_t position */

    char *name;

    bool isOpen;

    char *buffer;

    /**
     * file pointer = buffer_start + buffer_position
     */
    apr_off_t  buffer_start;      /* position in file of buffer */
    apr_off_t  buffer_position;   /* next byte to read          */
    unsigned int buffer_length;     /* end of valid bytes         */

    apr_status_t (*_length) ( lcn_ostream_t *ostream, apr_off_t * off);


    /**
     * Returns LUCENE_OK on success, LUCENE_IOERR on failure
     */
    apr_status_t (*_seek) ( lcn_ostream_t *ostream, apr_off_t off);

    /**
     * An abstract method, to be overriden by concrete classes.
     */
    apr_status_t (*_flush_buffer) ( lcn_ostream_t *ostream,
                                    char *buffer,
                                    size_t buf_size );

}  ;

#endif /* OSTREAM_H */
