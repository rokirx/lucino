#include "lucenecpp.hpp"
#include "apr_pools.h"
#include "exception.hpp"

#pragma warning( disable : 4290 )

NAMESPACE_LCN_BEGIN

apr_status_t lastError;

void Initialize()
{
    apr_pool_t* basePool;

    apr_initialize();

    if( APR_SUCCESS != apr_pool_create( &basePool, NULL ) )
    {
        LCN_THROW_MEMORY_EXCEPTION();
    }

    if( APR_SUCCESS != lcn_atom_init( basePool ) )
    {
        LCN_THROW_MEMORY_EXCEPTION();
    }
}

void Terminate()
{
    apr_terminate();
}

NAMESPACE_LCN_END

