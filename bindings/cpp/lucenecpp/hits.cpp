#include "hits.hpp"

NAMESPACE_LCN_BEGIN

Hits::Hits()
    : PoolObject<lcn_hits_t>()
{}

int
Hits::length()
{
    return lcn_hits_length( *this );
}

Document
Hits::doc( int i ) throw( IOException )
{
    Document result;
    apr_status_t rv;

    if( ( rv = lcn_hits_doc( *this, &result, i, result.pool() ) ) )
    {
        result.clear();

	LCN_THROW_IO_EXCEPTION( rv, "Error getting Document" );
    }

    return result;
}


NAMESPACE_LCN_END


