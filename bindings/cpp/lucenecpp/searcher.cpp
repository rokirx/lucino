#include "searcher.hpp"

NAMESPACE_LCN_BEGIN

Hits 
Searcher::search( const Query& query ) throw ( IOException )
{
    Hits result;
    apr_status_t rv;

    
    result._query = query.clone();

    if( rv = lcn_searcher_search( *this, &result, result._query, NULL, 
				  result.pool() ) )
    {
	LCN_THROW_IO_EXCEPTION( rv, "Error executing Query" );
    }
    return result;
}


NAMESPACE_LCN_END

