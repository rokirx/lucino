#include "string_buffer.h"
#include <stdarg.h>


apr_status_t
lcn_string_buffer_to_string( lcn_string_buffer_t* string_buffer,
                             char** result,
                             apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i, str_size = 0, act_off = 0;

        for( i = 0; i < lcn_list_size( string_buffer->string_list ); i++ )
        {
            lcn_string_buffer_entry_t* entry = lcn_list_get( string_buffer->string_list, i );
            str_size += entry->len;
        }

        LCNCE( s );

        LCNPV( *result = apr_palloc( pool, str_size+1 ), APR_ENOMEM );

        for( i = 0; i < lcn_list_size( string_buffer->string_list ); i++ )
        {
            lcn_string_buffer_entry_t* entry = lcn_list_get( string_buffer->string_list, i );
            memcpy( ( (*result) + act_off ), entry->str, entry->len );
            act_off += entry->len;
        }

        LCNCE( s );

        (*result)[str_size] = 0;
    }
    while( FALSE );

    return s;
}


static apr_status_t
lcn_string_buffer_append_entry( lcn_string_buffer_t* sb )
{
    apr_status_t s;
    apr_pool_t* pool;
    lcn_list_t* list;
    lcn_string_buffer_entry_t* entry;

    list = sb->string_list;
    pool = lcn_list_pool( list );

    LCNPR( entry = apr_pcalloc( pool,
                                sizeof( lcn_string_buffer_entry_t ) ),
           APR_ENOMEM );

    LCNPR( entry->str = apr_palloc( pool,
                                    sb->block_size ),
           APR_ENOMEM );

    LCNCR( lcn_list_add( sb->string_list, entry ) );

    sb->last = entry;

    return s;
}

static unsigned int
lcn_string_buffer_append_internal( lcn_string_buffer_t* sb,
                                   const char* string,
                                   unsigned int max )
{
    char c;
    unsigned int bytes_written = 0;

    lcn_string_buffer_entry_t* entry;

    entry = sb->last;

    while( ( entry->len < sb->block_size ) &&
           ( c = string[bytes_written] ) &&
           ( bytes_written < max ) )
    {
        bytes_written++;
        entry->str[entry->len++] = c;
    }

    return bytes_written;
}

apr_status_t
lcn_string_buffer_append( lcn_string_buffer_t* sb,
                          const char* string )
{
    return lcn_string_buffer_nappend( sb, string, strlen( string ));
}

apr_status_t
lcn_string_buffer_nappend( lcn_string_buffer_t* sb,
                           const char* string,
                           unsigned int n )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int len, bytes_written = 0;
        lcn_list_t* blocks;

        len = strlen( string );

        if( n == 0 || len == 0)
        {
            return APR_SUCCESS;
        }

        len = (len < n ? len : n );

        blocks = sb->string_list;

        if( lcn_list_size( blocks ) == 0 )
        {
            LCNCE( lcn_string_buffer_append_entry( sb ) );
        }

        while( bytes_written < len )
        {
            unsigned int cur_written =
                lcn_string_buffer_append_internal( sb, ( string + bytes_written ),
                                                   ( len - bytes_written ) );

            if( sb->last->len >= sb->block_size )
            {
                LCNCE( lcn_string_buffer_append_entry( sb ) );
            }

            bytes_written += cur_written;
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_string_buffer_append_float( lcn_string_buffer_t* string_buffer,
                                float f )
{
    char buf[100];
    apr_snprintf( buf, 100, "%f", f );
    return lcn_string_buffer_append( string_buffer, buf );
}

apr_status_t
lcn_string_buffer_append_int( lcn_string_buffer_t* string_buffer,
                             int i )
{
    char buf[20];
    apr_snprintf( buf, 20, "%d", i );
    return lcn_string_buffer_append( string_buffer, buf );
}

unsigned int
lcn_string_buffer_length( const lcn_string_buffer_t* sb )
{
    unsigned int i, result = 0;

    for( i = 0; i < lcn_list_size( sb->string_list ); i++ )
    {
        lcn_string_buffer_entry_t* en = lcn_list_get( sb->string_list, i  );
        result += en->len;
    }

    return result;
}

apr_status_t
lcn_string_buffer_append_uint( lcn_string_buffer_t* string_buffer,
                               unsigned int i )
{
    char buf[20];
    apr_snprintf( buf, 20, "%u", i );
    return lcn_string_buffer_append( string_buffer, buf );
}

apr_status_t
lcn_string_buffer_append_buffer( lcn_string_buffer_t* string_buffer,
                                 lcn_string_buffer_t* to_add )
{
    apr_status_t s;

    do
    {
        apr_pool_t* pool;
        char* str_to_add;

        LCNCE( apr_pool_create( &pool,
                                lcn_list_pool( string_buffer->string_list ) ) );

        LCNCE( lcn_string_buffer_to_string( to_add, &str_to_add, pool ) );

        LCNCE( lcn_string_buffer_append( string_buffer, str_to_add ) );

        apr_pool_destroy( pool );
    }
    while( FALSE );

    return s;
}

char
lcn_string_buffer_char_at( const lcn_string_buffer_t* sb, unsigned int i )
{
    unsigned int offset;

    if( i > lcn_string_buffer_length( sb ) )
    {
        return 0;
    }

    offset = ( i % sb->block_size );

    return sb->last->str[offset];
}

char
lcn_string_buffer_last_char( const lcn_string_buffer_t* sb )
{
    return lcn_string_buffer_char_at( sb, lcn_string_buffer_length( sb ) - 1 );
}

void
lcn_debug_no_out( FILE *stream, const char* fmt, ... ){}

apr_status_t
lcn_string_buffer_append_format( lcn_string_buffer_t* sb,
                                 const char* fmt, ... )
{
    apr_status_t s;
    apr_pool_t* pool;
    va_list ap;
    char *to_add;

    LCNCR( apr_pool_create( &pool, lcn_list_pool( sb->string_list ) ) );
    va_start(ap, fmt);
    to_add = apr_pvsprintf(pool, fmt, ap);
    va_end(ap);
    LCNCR( lcn_string_buffer_append( sb, to_add ) );

    apr_pool_destroy( pool );

    return s;
}

apr_status_t
lcn_string_buffer_create( lcn_string_buffer_t** string_buffer,
                          apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *string_buffer = apr_pcalloc( pool,
                                             sizeof( lcn_string_buffer_t ) ),
               APR_ENOMEM );

        (*string_buffer)->block_size = LCN_STRING_BUFFER_BLOCK_SIZE;
        LCNCE( lcn_list_create( &((*string_buffer)->string_list),
                                10,
                                pool ) );
    }
    while( FALSE );

    return s;
}

apr_pool_t*
lcn_string_buffer_pool( lcn_string_buffer_t* buf )
{
    return lcn_list_pool( buf->string_list );
}
