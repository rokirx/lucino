#include "lucene.h"


BEGIN_C_DECLS

struct lcn_list_t{

    /**
     * apr_pool
     */
    apr_pool_t *pool;

    /**
     * size of the list
     */
    unsigned int size;
    unsigned int size_log;
    unsigned int block_size;
    void ***blocks;
    unsigned int blocks_number;
    unsigned int blocks_buffer_size;
};

apr_pool_t *
lcn_list_pool( lcn_list_t *list )
{
    return list->pool;
}

void
lcn_list_clear( lcn_list_t *list )
{
    list->size = 0;
}

unsigned int
lcn_list_size( const lcn_list_t *list )
{
    return list->size;
}

void
lcn_list_remove( lcn_list_t *list, unsigned int n )
{
    unsigned int i;

    if( n < list->size )
    {
        for( i = n; i < ( list->size - 1); i++ )
        {
            unsigned int j;
            unsigned int act_bs;

            /* current block */
            act_bs = i / list->block_size;

            /* current element in block */
            j = i;
            j = j % list->block_size;

            /* all blocks are shifted to left */
            if( j < list->block_size - 1 )
            {
                list->blocks[ act_bs ][ j ] = list->blocks[ act_bs ][ j + 1 ];
            }
            /* first element of the next block becomes last element of the current block */
            else
            {
                list->blocks[ act_bs ][ j ] = list->blocks[ act_bs + 1 ][ 0 ];
            }
        }

        list->size--;
    }
}

apr_status_t
lcn_list_add( lcn_list_t *list, void *element )
{
    size_t i;

    if ( list->size >= list->blocks_number * list->block_size )
    {
        if ( list->blocks_number == list->blocks_buffer_size )
        {
            void ***tmp;

            list->blocks_buffer_size *= 2;

            tmp = (void ***) apr_palloc( list->pool, sizeof(void **) * list->blocks_buffer_size );

            for ( i=0; i < list->blocks_number; i++ )
            {
                tmp[i] = list->blocks[i];
            }

            list->blocks = tmp;
        }

        list->blocks_number++;
        list->blocks[ list->blocks_number - 1 ] = (void **) apr_palloc( list->pool, sizeof(void *) * list->block_size );
    }

    list->blocks[ list->size / list->block_size ] [ list->size & (list->block_size - 1) ] = element;
    list->size++;

    return APR_SUCCESS;
}

void*
lcn_list_get( const lcn_list_t *list, unsigned int n )
{
    if( n >= list->size )
    {
        return NULL;
    }

    return list->blocks[ n >> list->size_log ] [ n & (list->block_size - 1) ];
}

void
lcn_list_set ( lcn_list_t *list, unsigned int n, void *element  )
{
    int block = n >> list->size_log;
    list->blocks[ block ] [ n - (block << list->size_log) ] = element;
}

/**
 * Create lcn_list_t with 2^log2(size) block length. It is the
 * initial size of the underlying array. If the list grows
 * additional array of the same size is allocated.
 */
apr_status_t
lcn_list_create ( lcn_list_t **new_list, unsigned int size, apr_pool_t *pool )
{
    apr_status_t s;
    int size_log = 0;

    do
    {
        LCNPV( *new_list = (lcn_list_t *) apr_pcalloc( pool, sizeof(lcn_list_t) ), APR_ENOMEM );

        while ( size > 0 )
        {
            size_log++;
            size = size >> 1;
        }

        (*new_list)->pool = pool;
        (*new_list)->block_size = 1 << size_log;
        (*new_list)->size_log = size_log;
        (*new_list)->blocks_number = 1;
        (*new_list)->blocks_buffer_size = 1;

        LCNPV( (*new_list)->blocks = (void ***) apr_palloc( pool, sizeof(void **) * (*new_list)->blocks_number ),
               APR_ENOMEM );

        LCNPV( (*new_list)->blocks[0] = (void **) apr_palloc( pool, sizeof(void *) * (*new_list)->block_size ),
               APR_ENOMEM );
    }
    while(0);

    return s;
}

apr_status_t
lcn_list_swap( lcn_list_t *list,
               unsigned int a,
               unsigned int b )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        void *temp;

        LCNASSERT( (a < list->size && b < list->size), LCN_ERR_INVALID_ARGUMENT );

        /* Block von a und b ermitteln */
        temp = list->blocks[ a >> list->size_log ] [ a & (list->block_size - 1) ];
        list->blocks[ a >> list->size_log ] [ a & (list->block_size - 1) ] =
            list->blocks[ b >> list->size_log ] [ b & (list->block_size - 1) ];

        list->blocks[ b >> list->size_log ] [ b & (list->block_size - 1) ] = temp;
    }
    while( 0 );

    return s;
}

apr_status_t
lcn_list_sort( lcn_list_t* list,
               int(*sort_func)(const void* a, const void* b ) )
{
    apr_status_t s;
    apr_pool_t* cp;
    void** elements;
    unsigned int i;
    unsigned int list_size = lcn_list_size( list );

    LCNCR( apr_pool_create( &cp, list->pool ) );
    LCNPR( elements = apr_palloc( cp, list_size * sizeof( void*) ),
           APR_ENOMEM );

    for( i = 0; i < list_size; i++ )
    {
        elements[i] = lcn_list_get( list, i );
    }

    list->size = 0;

    qsort( elements, list_size, sizeof( void* ), sort_func );

    for( i = 0; i < list_size; i++ )
    {
        LCNCE( lcn_list_add( list, elements[i] ) );
    }

    apr_pool_destroy( cp );

    return s;
}


static int
sort_cstrings( const void* a, const void* b )
{
    char * const *sa = a;
    char * const *sb = b;
    return strcmp( *sa, *sb );
}

  apr_status_t
lcn_list_sort_cstrings( lcn_list_t *list )
{
    return lcn_list_sort( list, sort_cstrings );
}


apr_status_t
lcn_list_insert( lcn_list_t *list, unsigned int pos, void* element )
{
    apr_status_t s;
    unsigned int i;

    LCNCR( lcn_list_add( list, element ) );

    if( lcn_list_size( list ) == 1 )
    {
        return s;
    }

    for( i = lcn_list_size( list ) - 1; i > pos; i-- )
    {
        lcn_list_set( list, i, lcn_list_get( list, i - 1 ) );
    }

    lcn_list_set( list, pos, element );

    return s;
}

apr_status_t
lcn_list_append( lcn_list_t *list, lcn_list_t *list_to_add )
{
    apr_status_t s;
    unsigned int i;

    for( i = 0; i < lcn_list_size( list_to_add ); i++ )
    {
        LCNCR( lcn_list_add( list, lcn_list_get( list_to_add, i ) ) );
    }

    return APR_SUCCESS;
}


apr_status_t
lcn_list_uniquify( lcn_list_t **new_list,
                   lcn_list_t *list,
                   apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    do
    {
        apr_hash_t* ht;
        apr_hash_index_t* hi;
        unsigned int i;

        LCNCE( apr_pool_create( &cp, pool ) );

        ht = apr_hash_make( cp );
        LCNCE( lcn_list_create( new_list, lcn_list_size( list ), pool ) );

        for( i = 0; i < lcn_list_size( list ); i++ )
        {
            char *value = lcn_list_get( list, i );

            if( apr_hash_get( ht, value, APR_HASH_KEY_STRING ) == NULL )
            {
                apr_hash_set( ht, value, APR_HASH_KEY_STRING, "0" );
            }
        }

        for( hi = apr_hash_first( cp, ht ); hi; hi = apr_hash_next( hi ) )
        {
            void *val;
            const char *key;
            apr_ssize_t len;
            apr_hash_this(hi, (void*)&key, &len, &val);

            if( strcmp((const char*)val, "0") == 0 )
            {
                LCNCE( lcn_list_add( *new_list,
                                     (void*)apr_pstrndup( pool,
                                                          key,
                                                          strlen( key ) + 1 ) ) );
            }
        }
    }
    while( 0 );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

void*
lcn_list_last( lcn_list_t* list )
{
    void *result = NULL;

    if( list->size > 0 )
    {
        result = lcn_list_get( list, (list->size - 1) );
    }

    return result;
}


END_C_DECLS
