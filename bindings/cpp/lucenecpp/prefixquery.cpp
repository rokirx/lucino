#include "prefixquery.hpp"

NAMESPACE_LCN_BEGIN

PrefixQuery::PrefixQuery( const Term& term )
  : Query()
{
    lcn_prefix_query_create( &(*this), term, pool() );
    assertNotNull();
}

PrefixQuery::~PrefixQuery()
{
}

NAMESPACE_LCN_END
