#include "query.hpp"
#include <stdio.h>

NAMESPACE_LCN_BEGIN

Query::Query() : PoolObject<lcn_query_t>()
{}

Query 
Query::clone() const
{
    Query query;

    lcn_query_clone( (*this), &query, query.pool() );

    query.assertNotNull();

    return query;
}

String
Query::toString( const String& field ) const
{
    AutoReleasePool pool;
    char* c_result;

    lcn_query_t* query = (lcn_query_t*)(*this);

    if( APR_ENOMEM == 
	lcn_query_to_string( (*this), &c_result, field, pool ) )
    {
	LCN_THROW_MEMORY_EXCEPTION();
    }

    return String( c_result );
}


NAMESPACE_LCN_END
