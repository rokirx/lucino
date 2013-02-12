#include "lcn_search.h"
#include "priority_queue.h"

static void
lcn_priority_queue_up_heap( lcn_priority_queue_t *queue )
{
    unsigned int i = queue->size;
    void *node = queue->heap[i];                     /* save bottom node */
    unsigned int j = i >> 1;

    while (j > 0 && queue->less_than( queue, node, queue->heap[j] ) )
    {
        queue->heap[i] = queue->heap[j];
        i = j;
        j = j >> 1;
    }

    queue->heap[i] = node;                           /* install saved node */
}

static void
lcn_priority_queue_down_heap( lcn_priority_queue_t *queue )
{
    unsigned int i = 1;
    void *node = queue->heap[i];            /* save top node      */
    unsigned int j = i << 1;                /* find smaller child */
    unsigned int k = j + 1;


    if ( k <= queue->size && queue->less_than( queue, queue->heap[k], queue->heap[j] ) )
    {
        j = k;
    }

    while ( j <= queue->size && queue->less_than( queue, queue->heap[j], node) )
    {

        queue->heap[i] = queue->heap[j];          /* shift up child */
        i = j;
        j = i << 1;
        k = j + 1;

        if ( k <= queue->size && queue->less_than( queue, queue->heap[k], queue->heap[j]) )
        {
            j = k;
        }

    }
    queue->heap[i] = node;                        /* install saved node */
}

void*
lcn_priority_queue_top( lcn_priority_queue_t *queue )
{
    return queue->size > 0 ? queue->heap[1] : NULL;
}

void*
lcn_priority_queue_element_at( lcn_priority_queue_t* queue, unsigned int n )
{
    return queue->heap[ n ];
}

void
lcn_priority_queue_set_element_at( lcn_priority_queue_t* queue,
                                   void *element,
                                   unsigned int n )
{
    queue->heap[ n ] = element;
}

void*
lcn_priority_queue_max( lcn_priority_queue_t *queue )
{
    if ( queue->size > 0 )
    {
        int i, m = queue->size;

        for( i = queue->size - 1;  i > 0 ; i-- )
        {
            if ( queue->less_than( queue, queue->heap[m], queue->heap[i] ) )
            {
                m = i;
            }
        }

        return queue->heap[ m ];
    }

    return NULL;
}


apr_status_t
lcn_priority_queue_put( lcn_priority_queue_t *queue, void *element)
{

    if (queue->size >= queue->max_size)
    {
        return LCN_PRIORITY_QUEUE_MAX_SIZE_EXCEEDED;
    }

    queue->size++;
    queue->heap[ queue->size ] = element;
    lcn_priority_queue_up_heap( queue );
    return APR_SUCCESS;
}

apr_status_t
lcn_priority_queue_initialize( lcn_priority_queue_t *queue,
                               unsigned int max_size )
{
    apr_status_t s;

    do
    {
        queue->size = 0;
        queue->max_size = max_size;

        if ( NULL != queue->heap_pool )
        {
            apr_pool_clear( queue->heap_pool );
        }

        LCNCE( apr_pool_create( &(queue->heap_pool), queue->pool ) );
        LCNPV( queue->heap = apr_pcalloc( queue->heap_pool, sizeof(void*)*( 1 + queue->max_size ) ), APR_ENOMEM );
    }
    while( FALSE );

    return s;
}

void *
lcn_priority_queue_insert( lcn_priority_queue_t *queue, void *element)
{
    void *el = NULL;

    if ( queue->size < queue->max_size)
    {
        lcn_priority_queue_put( queue, element);
        return (void*) NULL;
    }
    else if ( queue->size > 0 &&
              /* if queue->size > 0 it is safe to assume */
              /* that top returns not null               */
              ! queue->less_than( queue, element, (el = lcn_priority_queue_top( queue )) ) )
    {
        queue->heap[ 1 ] = element;
        lcn_priority_queue_down_heap( queue );
        return el;
    }

    return element;
}

unsigned int
lcn_priority_queue_max_size( lcn_priority_queue_t* queue )
{
    return queue->max_size;
}

void*
lcn_priority_queue_pop( lcn_priority_queue_t *queue )
{
    void *result;

    if ( queue->size > 0)
    {
        result = queue->heap[1];                     /* save first value     */
        queue->heap[1] = queue->heap[ queue->size ]; /* move last to first   */
        queue->size--;

        /* adjust heap */
        lcn_priority_queue_down_heap( queue );

        return result;
    }

    return NULL;
}

void
lcn_priority_queue_clear( lcn_priority_queue_t *queue )
{
    int i;

    /* http://svn.apache.org/viewvc/lucene/dev/trunk/lucene/core/src/java/org/apache/lucene/util/PriorityQueue.java?revision=1386681&view=markup line 220 */

    for(i=0; i <= queue->size; i++ )
    {
        if( queue->heap != NULL )
        {
            queue->heap[i] = NULL;
        }
    }

    queue->size = 0;
}

unsigned int
lcn_priority_queue_size( lcn_priority_queue_t* queue )
{
    return queue->size;
}

void
lcn_priority_queue_adjust_top( lcn_priority_queue_t* queue )
{
    lcn_priority_queue_down_heap( queue );
}

apr_status_t
lcn_priority_queue_create( lcn_priority_queue_t** priority_queue,
                           lcn_bool_t (*less_than)( lcn_priority_queue_t*, void*, void* ),
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( (*priority_queue) = apr_pcalloc( pool, sizeof( lcn_priority_queue_t ) ), APR_ENOMEM );
        LCNCE( apr_pool_create( &((*priority_queue)->pool), pool ) );

        (*priority_queue)->less_than = less_than;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_priority_queue_resize( lcn_priority_queue_t *priority_queue,
                           size_t new_size )
{
    apr_status_t s = APR_SUCCESS;

    if ( new_size <= priority_queue->max_size )
    {
        return s;
    }

    do
    {
        apr_pool_t *p;
        void **new_heap;

        lcn_priority_queue_clear( priority_queue );

        LCNCE( apr_pool_create( &p, priority_queue->pool ));
        LCNPV( new_heap = apr_pcalloc( p, sizeof(void*)*( 1 + new_size ) ), APR_ENOMEM );

        memcpy( new_heap, priority_queue->heap, 1 + priority_queue->max_size );

        priority_queue->heap = new_heap;
        priority_queue->max_size = new_size;

        if ( NULL != priority_queue->heap_pool )
        {
            apr_pool_destroy( priority_queue->heap_pool );
        }

        priority_queue->heap_pool = p;
    }
    while( FALSE );

    return s;

}
