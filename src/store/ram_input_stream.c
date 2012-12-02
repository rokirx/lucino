#include "index_input.h"
#include "lcn_store.h"


struct ram_input_stream {

    lcn_index_input_t index_input;

    lcn_ram_file_t *file;
};



static apr_status_t
lcn_index_input_ram_read_internal( lcn_index_input_t *in,
                                   char *dest,
                                   apr_off_t offset,
                                   unsigned int len)
{
    unsigned int remainder = len;
    apr_off_t start = in->pointer;

    unsigned int buffer_number;
    apr_off_t buffer_offset;
    unsigned int bytes_in_buffer;
    unsigned int bytes_to_copy;
    char *file_buffer;

    struct ram_input_stream* ram_is = (struct ram_input_stream*) in;

    while (remainder)
    {
        buffer_number = (unsigned int)(start / LCN_STREAM_BUFFER_SIZE);
        buffer_offset = start % LCN_STREAM_BUFFER_SIZE;
        bytes_in_buffer = (unsigned int)(LCN_STREAM_BUFFER_SIZE - buffer_offset);
        bytes_to_copy = bytes_in_buffer >= remainder ? remainder : bytes_in_buffer;

        file_buffer = lcn_list_get( ram_is->file->buffers,
                                    (unsigned int) buffer_number );

        memcpy( dest + offset, file_buffer + buffer_offset, bytes_to_copy );
        offset += bytes_to_copy;
        start += bytes_to_copy;
        remainder -= bytes_to_copy;
    }

    in->position = in->pointer += len;

    return APR_SUCCESS;
}

static apr_status_t
lcn_index_input_ram_clone( lcn_index_input_t *in,
                           lcn_index_input_t **clone_in,
                           apr_pool_t *pool )
{
    apr_status_t s;

    struct ram_input_stream* ram_is = (struct ram_input_stream*) in;

    LCNCR( lcn_ram_input_stream_create( clone_in, NULL, ram_is->file, pool ));

    (*clone_in)->size     = in->size;
    (*clone_in)->is_open  = in->is_open;
    (*clone_in)->is_clone = LCN_TRUE;

    return s;
}

static apr_status_t
lcn_index_input_ram_close( lcn_index_input_t *istream )
{
    istream->is_open = LCN_FALSE;
    return APR_SUCCESS;
}

apr_status_t
lcn_ram_input_stream_create( lcn_index_input_t **new_in,
                             const char *name,
                             lcn_ram_file_t *file,
                             apr_pool_t *pool )
{
    apr_status_t s;
    struct ram_input_stream *is;

    do
    {
        LCNPV( is = (struct ram_input_stream*) apr_pcalloc( pool, sizeof(struct ram_input_stream)), APR_ENOMEM );
        *new_in = (lcn_index_input_t*) is;

        LCNCE( lcn_index_input_init( *new_in, name, pool ) );

        (*new_in)->pointer = 0;
        (*new_in)->size = file->length;

        is->file = file;

        /* overload implementation specific methods */

        (*new_in)->_close         = lcn_index_input_ram_close;
        (*new_in)->_read_internal = lcn_index_input_ram_read_internal;
        (*new_in)->_clone         = lcn_index_input_ram_clone;
    }
    while(0);

    return s;
}
