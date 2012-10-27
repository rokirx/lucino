#include "autoreleasepool.hpp"
#include "lcn_util.h"
#include "exception.hpp"
NAMESPACE_LCN_BEGIN


#define POOL_DEBUG_SET 0

#if POOL_DEBUG_SET
#define POOL_DEBUG( STR, PTR ) cerr << STR << (void*)PTR << endl;
#else
#define POOL_DEBUG( STR, PTR )
#endif

int poolCount;

AutoReleasePool::AutoReleasePool()
{
    createPool();

}

AutoReleasePool::AutoReleasePool( apr_pool_t* parent)
{
    createPool( parent );
}

AutoReleasePool::AutoReleasePool( const AutoReleasePool& other )
{
    createPool();

    (*this) = other;

}


AutoReleasePool::~AutoReleasePool()
{
    (*_refCount)--;

    if( 0 == (*_refCount) )
    {
        destroyPool();
    }
}

AutoReleasePool& 
AutoReleasePool::operator=( const AutoReleasePool& other )
{
    if( _pool == other._pool )
    {
        return *this;
    }
    else
    {
        (*_refCount)--;

        if( 0 == (*_refCount) )
        {
            destroyPool();
        }

        _refCount = other._refCount;
        _pool     = other._pool;

        (*_refCount)++;
    }

    return *this;
}

int
AutoReleasePool::refCount() const
{
    return *_refCount;
}

apr_pool_t* 
AutoReleasePool::pool()
{
    return _pool;
}

void 
AutoReleasePool::clear()
{
    if( 1 == (*_refCount) )
    {
        clearPool();
    }
    else if( 1 < (*_refCount ) )
    {
        (*_refCount)--;
        createPool();
    }
}

AutoReleasePool::operator apr_pool_t*()
{
    return _pool;
}

void
AutoReleasePool::createPool( apr_pool_t* parent )
{
    apr_pool_create( &_pool, parent );

    if( NULL == _pool )
    {
        LCN_THROW_MEMORY_EXCEPTION();
    }
    _refCount = (int*)apr_pcalloc( _pool, sizeof( int ) );
    (*_refCount) = 1;
}

void
AutoReleasePool::destroyPool()
{
    apr_pool_destroy( _pool );
}

void
AutoReleasePool::clearPool()
{
    apr_pool_clear( _pool );
    _refCount = (int*)apr_pcalloc( _pool, sizeof( int ) );
    (*_refCount) = 1;
}
NAMESPACE_LCN_END

