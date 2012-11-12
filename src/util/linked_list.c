#include "lcn_util.h"

struct lcn_linked_list_el_t
{
    lcn_linked_list_el_t* next;
    lcn_linked_list_el_t* previous;
    void* content;
};

struct lcn_linked_list_t
{
    lcn_linked_list_el_t* first;
    lcn_linked_list_el_t* last;

    unsigned int size;

    apr_pool_t* pool;
    apr_pool_t* el_pool;
};

static apr_status_t
lcn_linked_list_el_create( lcn_linked_list_el_t** el, void* content, apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *el = apr_pcalloc( pool, sizeof( lcn_linked_list_el_t ) ),
               APR_ENOMEM );

        (*el)->content = content;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_linked_list_create( lcn_linked_list_t** l, apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *l = apr_pcalloc( pool, sizeof( lcn_linked_list_t ) ),
               APR_ENOMEM );

        (*l)->pool = pool;

        LCNCE( apr_pool_create( &((*l)->el_pool ), pool ) );
    }
    while( FALSE );

    return s;
}

unsigned int
lcn_linked_list_size( lcn_linked_list_t* l )
{
    return l->size;
}

void
lcn_linked_list_clear( lcn_linked_list_t* l )
{
    l->first = NULL;
    l->last  = NULL;

    l->size = 0;

    apr_pool_clear( l->el_pool );
}

apr_status_t
lcn_linked_list_add_last( lcn_linked_list_t* l,
                          void* content )
{
    apr_status_t s;

    do
    {
        lcn_linked_list_el_t* el;

        LCNCE( lcn_linked_list_el_create( &el, content, l->el_pool ) );

        if( NULL == l->last && NULL == l->first )
        {
            l->first = el;
            l->last = el;
        }
        else
        {
            l->last->next = el;
            el->previous = l->last;
            l->last = el;
        }

        l->size++;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_linked_list_add_first( lcn_linked_list_t* l,
                           void* content )
{
     apr_status_t s;

    do
    {
        lcn_linked_list_el_t* el;

        LCNCE( lcn_linked_list_el_create( &el, content, l->el_pool ) );

        if( NULL == l->last && NULL == l->first )
        {
            l->first = el;
            l->last = el;
        }
        else
        {
            el->next = l->first;

            if( l->first )
            {
                l->first->previous = el;
            }

            l->first = el;
        }
        l->size++;
    }
    while( FALSE );

    return s;
}

const lcn_linked_list_el_t*
lcn_linked_list_remove_first( lcn_linked_list_t* l )
{
    lcn_linked_list_el_t* sav;

    if( l->size == 0 )
    {
        return NULL;
    }

    sav = l->first;

    l->first = l->first->next;

    if( l->first )
    {
        l->first->previous = NULL;
    }

    l->size--;

    return sav;
}

const lcn_linked_list_el_t*
lcn_linked_list_remove_element( lcn_linked_list_t* l,
                                const lcn_linked_list_el_t *el )
{
    if ( l->first == el )
    {
        return lcn_linked_list_remove_first( l );
    }

    if ( l->last == el )
    {
        return lcn_linked_list_remove_last( l );
    }

    el->next->previous = el->previous;
    el->previous->next = el->next;

    l->size--;

    return el;
}

const lcn_linked_list_el_t*
lcn_linked_list_remove_last( lcn_linked_list_t* l )
{
    lcn_linked_list_el_t* el, *sav;

    if( l->size == 0 )
    {
        return NULL;
    }

    sav = l->last;
    el = l->last->previous;

    if( el == NULL )
    {
        l->first = NULL;
        l->last  = NULL;
    }
    else
    {
        el->next = NULL;
        l->last = el;
    }

    l->size--;

    return sav;
}

const lcn_linked_list_el_t*
lcn_linked_list_first( lcn_linked_list_t* l )
{
    return l->first;
}

const lcn_linked_list_el_t*
lcn_linked_list_last( lcn_linked_list_t* l )
{
    return l->last;
}

const lcn_linked_list_el_t*
lcn_linked_list_next( const lcn_linked_list_el_t* l )
{
    return l->next;
}


void*
lcn_linked_list_content( const lcn_linked_list_el_t* el )
{
    return el->content;
}

lcn_linked_list_el_t*
lcn_linked_list_get_element( lcn_linked_list_t* l,
                             unsigned int nth )
{
    int i;
    lcn_linked_list_el_t* el;

    el = l->first;

    for(i = 1;i < nth;i++)
    {
        el = (lcn_linked_list_el_t*) lcn_linked_list_next(el);
    }

    return el;
}

apr_status_t
lcn_linked_list_to_array( lcn_linked_list_t* l,
                          lcn_ptr_array_t** array,
                          apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        const lcn_linked_list_el_t* el;
        unsigned int i = 0;

        LCNCE( lcn_ptr_array_create( array,
                                     lcn_linked_list_size( l ),
                                     pool ) );

        for( el = lcn_linked_list_first( l );
             el != NULL;
             el = lcn_linked_list_next( el ) )
        {
            (*array)->arr[i++] = lcn_linked_list_content( el );
        }
    }
    while( FALSE );

    return s;
}
