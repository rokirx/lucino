#include "lucene.h"
#include "lcn_store.h"
#include "lcn_util.h"

#include "istream.h"
#include "lcn_bitvector.h"


BEGIN_C_DECLS


struct lcn_istream_t {

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
    apr_status_t (*_read_internal) ( lcn_istream_t *,
                                     char *dest,
                                     apr_off_t offset,
                                     unsigned int len);

    /**
     * concrete implementation of the clone method
     *
     * @param istream The base InputStream
     * @param clone_is The cloned InputStream
     */
    apr_status_t (*_clone) ( lcn_istream_t *istream,
                             lcn_istream_t **clone,
                             apr_pool_t *pool );

    apr_status_t (*_close)( lcn_istream_t *istream );


    /* RAMInputStream */
    apr_off_t pointer;
    lcn_ram_file_t *_file;

    /* CSInputStream */
    lcn_istream_t *base;
    apr_off_t file_offset;
};

apr_off_t
lcn_istream_file_pointer( lcn_istream_t *istream )
{
    return istream->buffer_start + istream->buffer_position;
}

apr_status_t
lcn_istream_close( lcn_istream_t *istream )
{
    return istream->_close( istream );
}

unsigned int
lcn_istream_size( lcn_istream_t *istream )
{
    return istream->size;
}

static apr_status_t
lcn_istream_fs_clone ( lcn_istream_t *istream,
                       lcn_istream_t **clone,
                       apr_pool_t *pool );

static apr_status_t
lcn_istream_fs_close( lcn_istream_t *istream )
{
    if ( ! istream->is_open )
    {
        return APR_SUCCESS;
    }

    istream->is_open = LCN_FALSE;

    if ( ! istream->is_clone )
    {
        return apr_file_close( istream->_file_desc );
    }

    return APR_SUCCESS;
}

static apr_status_t
lcn_istream_fs_read_internal ( lcn_istream_t *istream,
                               char *buffer,
                               apr_off_t offset,
                               unsigned int len )
{
    apr_status_t s;
    apr_size_t nbytes = len;

    do
    {
        apr_pool_t *pool;
        apr_pool_create( &pool, NULL );

        LCNCE( apr_file_seek( istream->_file_desc, APR_SET, &(istream->position) ) );
        LCNCE( apr_file_read( istream->_file_desc, buffer + offset, &nbytes ) );

        istream->position += (apr_off_t)len;
        apr_pool_destroy( pool );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_istream_init_base( lcn_istream_t **in,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *in = (lcn_istream_t*) apr_pcalloc( pool, sizeof(lcn_istream_t) ), APR_ENOMEM );
        LCNPV( (*in)->buffer = (char*) apr_palloc( pool, sizeof(char) * LCN_STREAM_BUFFER_SIZE), APR_ENOMEM );

        (*in)->_file = NULL;
        (*in)->name  = NULL;
        (*in)->position = 0;
        (*in)->is_open  = LCN_TRUE;
        (*in)->is_clone = LCN_FALSE;

        (*in)->buffer_start    = 0;
        (*in)->buffer_length   = 0;
        (*in)->buffer_position = 0;
        (*in)->pool = pool;

        (*in)->_clone         = lcn_istream_fs_clone;
        (*in)->_close         = lcn_istream_fs_close;
        (*in)->_read_internal = lcn_istream_fs_read_internal;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_istream_fs_clone ( lcn_istream_t *istream,
                       lcn_istream_t **clone,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_istream_init_base( clone, pool ) );

        (*clone)->position = 0;
        (*clone)->size     = istream->size;
        (*clone)->is_open  = istream->is_open;
        (*clone)->is_clone = LCN_TRUE;

        (*clone)->name = istream->name;
        (*clone)->_file_desc = istream->_file_desc;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_istream_ram_read_internal ( lcn_istream_t *in,
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

    while (remainder)
    {
        buffer_number = (unsigned int)(start / LCN_STREAM_BUFFER_SIZE);
        buffer_offset = start % LCN_STREAM_BUFFER_SIZE;
        bytes_in_buffer = (unsigned int)(LCN_STREAM_BUFFER_SIZE - buffer_offset);
        bytes_to_copy = bytes_in_buffer >= remainder ? remainder : bytes_in_buffer;

        file_buffer = lcn_list_get( in->_file->buffers,
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
lcn_istream_buf_read_internal( lcn_istream_t *in,
                               char *dest,
                               apr_off_t offset,
                               unsigned int len )
{
    in->position = (in->pointer += len);

    return APR_SUCCESS;
}


static apr_status_t
cs_read_internal( lcn_istream_t *in,
                  char *dest,
                  apr_off_t offset,
                  unsigned int len)
{
/* TODO: synchronized (base)  */

    apr_status_t s;
    apr_off_t start = in->buffer_start + in->buffer_position;

    if( (apr_off_t) (start + len) > in->size )
    {
        return LCN_ERR_READ_PAST_EOF;
    }

    do
    {
        LCNCE( lcn_istream_seek( in->base, in->file_offset + start ) );
        LCNCE( lcn_istream_read_bytes( in->base, dest, offset, &len ) );
    }
    while(0);

    return s;
}

static apr_status_t
refill ( lcn_istream_t *in )
{
    apr_status_t s;

    do
    {
        /* file pointer of the next byte to read */

        apr_off_t start = in->buffer_start + in->buffer_position;
        apr_off_t end;

        /* bytes which are not yet read and must be moved to the */
        /* beginning of the buffer                               */

        size_t rest = (size_t) (in->buffer_length - in->buffer_position );

        /* if the file pointer points to the end of file we are done */
        /* and cannot refill the buffer                              */

        if ( start == in->size )
        {
            return LCN_ERR_READ_PAST_EOF;
        }

        /* do not copy memory contents if the file is completely in */
        /* buffer.                                                  */

        if ( (apr_off_t) in->buffer_length > in->buffer_position  &&
             (apr_off_t) in->buffer_length == in->size )
        {
            return APR_SUCCESS;
        }

        if ( in->buffer_position == 0 && in->buffer_length == LCN_STREAM_BUFFER_SIZE )
        {
            return APR_SUCCESS;
        }

        if ( in->position == in->size )
        {
            return APR_SUCCESS;
        }

        /* if we have not read all contents of buffer, copy */
        /* the remaining byte to start                      */

        if ( rest > 0 )
        {
            if ( in->buffer_position > (apr_off_t) rest )
            {
                memcpy( in->buffer, in->buffer + in->buffer_position, rest );
            }
            else
            {
                size_t i = 0;
                char *source = in->buffer + in->buffer_position;

                while( i++ < rest )
                {
                    *(in->buffer++) = *(source++);
                }
            }

            start = in->buffer_start + in->buffer_position;
        }

        end = start + LCN_STREAM_BUFFER_SIZE;

        if ( end > in->size)  /* don't read past EOF */
        {
            end = (apr_off_t)(in->size);
        }

        in->buffer_length = (size_t) (end - start);

        if ( in->buffer_length == 0 && rest > 0 )
        {
            return APR_SUCCESS;
        }

        LCNASSERT( !( in->buffer_length == 0 ), LCN_ERR_READ_PAST_EOF );

        LCNCE( in->_read_internal( in, in->buffer, rest, (unsigned int) (in->buffer_length - rest)));

        in->buffer_start = start;
        in->buffer_position = 0;
    }
    while(0);

    return s;
}

apr_status_t
lcn_istream_read_byte (lcn_istream_t *in,
                       unsigned char *byte )
{
    if ( ((size_t) in->buffer_position) >= in->buffer_length)
    {
        apr_status_t s;

        if (( s = refill( in ) ))
        {
            return s;
        }
    }

    *byte = (unsigned char) in->buffer[ in->buffer_position++ ];

    return APR_SUCCESS;
}

apr_status_t
lcn_istream_read_bytes( lcn_istream_t *in,
                        char *bytes,
                        apr_off_t offset,
                        unsigned int *len )
{
    size_t i;
    apr_status_t s;

    if ( *len == 0 )
    {
        return APR_SUCCESS;
    }

    if ( *len < LCN_STREAM_BUFFER_SIZE)
    {
        /* read byte-by-byte */
        for ( i = 0; i < *len; i++ )
        {
            unsigned char b;

            if (( s = lcn_istream_read_byte( in, &b ) ))
            {
                *len = i;
                return LCN_ERR_READ_PAST_EOF;
            }

            bytes[i + offset] = (char) b;
        }
    } else {   /* read all-at-once */
        apr_off_t start = in->buffer_start + in->buffer_position;

        /* the next line is the inlined implementation of */
        /* seek_internal for RAM-lcn_istream_t            */

        in->pointer = start;
        in->position = start;

        if ((s = in->_read_internal( in, bytes, offset, *len )))
        {
            return s;
        }

        in->buffer_start = start + ((apr_off_t)*len);         /* adjust stream variables */
        in->buffer_position = 0;
        in->buffer_length = 0;                   /* trigger refill() on read */
    }

    return APR_SUCCESS;
}

apr_status_t
lcn_istream_read_int( lcn_istream_t *istream,
                      int* result )
{
    unsigned char *b;
    apr_status_t s;

    if ( 0 == istream->buffer_length ||
         ((size_t) istream->buffer_position + 4 ) > istream->buffer_length )
    {
        if (( s = refill( istream )))
        {
            return s;
        }
    }

    b = (unsigned char*) ( istream->buffer + istream->buffer_position );
    istream->buffer_position += 4;

    *result = (int) (((unsigned) b[0] << 24) |
                     ((unsigned) b[1] << 16) |
                     ((unsigned) b[2] << 8 ) |
                     ((unsigned) b[3] ));

    return APR_SUCCESS;
}

apr_status_t
lcn_istream_read_int16 ( lcn_istream_t *istream,
                         int *result )
{
    unsigned char *b;
    apr_status_t s;

    if ( 0 == istream->buffer_length ||
         ((size_t) istream->buffer_position + 2 ) > istream->buffer_length )
    {
        LCNCR( refill( istream ));
    }

    b = (unsigned char*) ( istream->buffer + istream->buffer_position );
    istream->buffer_position += 2;

    *result = (int) ( ((unsigned) b[0] << 8 ) | ((unsigned) b[1] ));

    return APR_SUCCESS;
}


apr_status_t
lcn_istream_read_vlong( lcn_istream_t *istream,
                        apr_uint64_t *result )
{
    apr_status_t s;
    apr_uint64_t i = 0;
    unsigned char b;
    int shift;

    LCNCR( lcn_istream_read_byte( istream, &b ));
    i = (apr_uint64_t) (((apr_uint64_t) b) & 0x7F);

    for ( shift = 7; (b & 0x80) != 0; shift += 7 )
    {
        LCNCE( lcn_istream_read_byte( istream, &b ));
        i |= (((apr_uint64_t) b) & 0x7f) << shift;
    }

    if ( APR_SUCCESS == s )
    {
        *result = i;
    }

    return s;
}


apr_status_t
lcn_istream_read_bitvector( lcn_istream_t *in,
                            lcn_bitvector_t **bitvector,
                            apr_pool_t *pool )
{
    apr_status_t s;
    unsigned int size;
    unsigned int len;
    char *bits;

    LCNCR( lcn_istream_read_vint( in, (unsigned int*)&size ) );

    len = (size>>3) + 1;

    if ( NULL == ( bits = apr_palloc( pool, sizeof(char) * len ) ))
    {
        return APR_ENOMEM;
    }

    LCNCR( lcn_istream_read_bytes( in, bits, 0, &len ));
    LCNCR( lcn_bitvector_create_by_bits( bitvector, size, bits, pool ));

    return s;
}


apr_status_t
lcn_istream_read_vint( lcn_istream_t *in,
                       unsigned int* result )
{
    apr_status_t s;
    unsigned char *buf;
    unsigned b;
    unsigned shift;
    unsigned i;

    if (! (bool) in->buffer_length || ((size_t) in->buffer_position + 5 ) > in->buffer_length )
    {
        LCNCR( refill( in ));
    }

    buf = (unsigned char *) (in->buffer + in->buffer_position);
    b = (unsigned) buf[0];
    *result = b & 0x7F;

    for (shift = 7, i = 1; (b & 0x80) != 0; i++, shift += 7 )
    {
        b = (unsigned) buf[i]; /* TODO */
        (*result) |= (b & 0x7F) << shift;
    }

    in->buffer_position += i;

    return APR_SUCCESS;
}


apr_status_t
lcn_istream_seek( lcn_istream_t *istream,
                  apr_off_t pos )
{
    if ( pos >= istream->buffer_start &&
         pos < (apr_off_t) ( istream->buffer_start + istream->buffer_length) )
    {
        /* seek within buffer */
        istream->buffer_position = ( pos - istream->buffer_start );
    } else {
        istream->buffer_start = pos;
        istream->buffer_position = 0;

        /* trigger refill() on read() */
        istream->buffer_length = 0;
        istream->position = pos; /* TODO: this differs from Java code but seems to be necessary here */
        istream->pointer = pos;
    }

    return APR_SUCCESS;
}

apr_status_t
lcn_istream_read_long( lcn_istream_t *in,
                       apr_int64_t *l )
{
    apr_status_t s;
    apr_int64_t hi, lo;
    int i;

    LCNCR( lcn_istream_read_int( in, &i ) );
    hi = (apr_int64_t) (unsigned) i;

    LCNCR( lcn_istream_read_int( in, &i ) );
    lo = (apr_int64_t) (unsigned) i;

    *l = (hi << 32 | lo );

    return s;
}

apr_status_t
lcn_istream_read_ulong( lcn_istream_t* istream,
                        apr_uint64_t* result )
{
    apr_status_t s;
    apr_int64_t value;

    LCNCR( lcn_istream_read_long( istream, &value ));
    *result = (apr_int64_t) value;

    return s;
}

apr_status_t
lcn_istream_read_chars ( lcn_istream_t *in,
                         char *buf,
                         apr_off_t start,
                         unsigned int length )
{
    apr_off_t end = start + length;
    apr_off_t i;
    unsigned b;
    apr_status_t s;
    buf[start] = '\0';

    for( i = start; i < end; i++)
    {
        if (! (bool) in->buffer_length || ((size_t) in->buffer_position + 5 ) > in->buffer_length )
        {
            LCNCE( refill( in ));
        }

        b = ((unsigned) *((unsigned char *) (in->buffer + in->buffer_position++)));

        if ((b & 0x80) == 0)
        {
            buf[i] = (char) (b & 0x7F);
        }
        else if ((b & 0xE0) != 0xE0)
        {
            buf[i] = (char) ((((unsigned) b & 0x1F) << 6) |
                             ( ((unsigned) *((unsigned char *) (in->buffer + in->buffer_position++))) & 0x3F));
        }
        else
        {
            buf[i] = (char) (((unsigned) (b & 0x0F) << 12) |
                             ((((unsigned) *((unsigned char *) (in->buffer + in->buffer_position+1))) & 0x3F) << 6) |
                             (( (unsigned) *((unsigned char *) (in->buffer + in->buffer_position+2))) & 0x3F));
            in->buffer_position+=2;
        }
    }

    return APR_SUCCESS;
}

apr_status_t
lcn_istream_read_string ( lcn_istream_t* in,
                          char **str,
                          unsigned int *len,
                          apr_pool_t *pool )
{
    apr_status_t s;
    unsigned int str_len, ui_str_len;

    do
    {
        LCNCE( lcn_istream_read_vint( in, &ui_str_len ) );
        str_len = ui_str_len;

        LCNPV( *str = (char*) apr_palloc( pool, (str_len + 1) * sizeof(char) ), APR_ENOMEM );

        *len = str_len;

        LCNCE( lcn_istream_read_chars( in, *str, 0, str_len ) );
        (*str)[str_len] = '\0';
    }
    while(0);

    return s;
}

static apr_status_t
lcn_istream_ram_close( lcn_istream_t *istream )
{
    istream->is_open = LCN_FALSE;
    return APR_SUCCESS;
}


static apr_status_t
lcn_istream_ram_clone( lcn_istream_t *istream,
                       lcn_istream_t **clone_in,
                       apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_ram_input_stream_create( clone_in, istream->_file, pool ));

    (*clone_in)->size     = istream->size;
    (*clone_in)->is_open  = istream->is_open;
    (*clone_in)->is_clone = LCN_TRUE;

    return s;
}

static apr_status_t
lcn_istream_buf_clone( lcn_istream_t *istream,
                       lcn_istream_t **clone_in,
                       apr_pool_t *pool )
{
    return LCN_ERR_UNSUPPORTED_OPERATION;
}


static apr_status_t
lcn_istream_cs_clone( lcn_istream_t *istream,
                      lcn_istream_t **clone,
                      apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_cs_input_stream_create ( clone,
                                        istream->base,
                                        istream->file_offset,
                                        istream->size,
                                        pool ) );

    (*clone)->is_open  = istream->is_open;
    (*clone)->is_clone = LCN_TRUE;

    return s;
}

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_istream_create) ( lcn_istream_t **new_is,
                                 const char *file_name,
                                 apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_finfo_t finfo;
        apr_file_t *newf;
        lcn_istream_t *is;

        LCNCE( lcn_istream_init_base( &is, pool ) );
        LCNCM( apr_stat( &finfo, file_name,
                         APR_FINFO_TYPE | APR_FINFO_SIZE,
                         is->pool ), file_name );

        LCNASSERT( finfo.filetype == APR_REG, LCN_ERR_NOT_REGULAR_FILE );

        LCNCM( apr_file_open( &newf, file_name,
                              APR_READ | APR_BINARY,
                              APR_OS_DEFAULT,
                              is->pool ), file_name );

        is->_file_desc = newf;
        is->size = finfo.size;
        is->position = 0;

        LCNPV( is->name = apr_pstrdup( is->pool, file_name ), APR_ENOMEM );

        *new_is = is;
    }
    while(0);

    return s;
}

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_ram_input_stream_create) ( lcn_istream_t **new_in,
                                          lcn_ram_file_t *file,
                                          apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_istream_init_base( new_in, pool ) );

    (*new_in)->pointer = 0;
    (*new_in)->_file = file;
    (*new_in)->size = file->length;

    /* overload implementation specific methods */

    (*new_in)->_close         = lcn_istream_ram_close;
    (*new_in)->_read_internal = lcn_istream_ram_read_internal;
    (*new_in)->_clone         = lcn_istream_ram_clone;

    return s;
}


apr_status_t
lcn_istream_buf_stream_create ( lcn_istream_t **new_in,
                                const char *buffer,
                                unsigned int len,
                                apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_istream_init_base( new_in, pool ) );

    (*new_in)->pointer = 0;
    (*new_in)->size    = len;
    (*new_in)->buffer  = (char*) buffer;

    /* overload implementation specific methods */

    (*new_in)->_close         = lcn_istream_ram_close;
    (*new_in)->_read_internal = lcn_istream_buf_read_internal;
    (*new_in)->_clone         = lcn_istream_buf_clone;

    return s;
}


LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_istream_clone) ( lcn_istream_t *istream,
                                lcn_istream_t **clone,
                                apr_pool_t *pool )
{
    return istream->_clone( istream, clone, pool );
}

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_cs_input_stream_create) ( lcn_istream_t **new_is,
                                         lcn_istream_t *base,
                                         apr_off_t file_offset,
                                         apr_off_t length,
                                         apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_istream_init_base( new_is, pool ) );

    (*new_is)->base = base;
    (*new_is)->file_offset = file_offset;
    (*new_is)->size = length;

    (*new_is)->_read_internal = cs_read_internal;
    (*new_is)->_close = lcn_istream_ram_close;
    (*new_is)->_clone = lcn_istream_cs_clone;

    return s;
}

apr_status_t
lcn_istream_name ( lcn_istream_t *istream,
                   char **name,
                   apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        LCNPV( (*name) = apr_pstrdup(pool, istream->name), APR_ENOMEM  );
    }
    while(0);

    return s;
}


END_C_DECLS