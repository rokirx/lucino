#include "termquery.hpp"

NAMESPACE_LCN_BEGIN

TermQuery::TermQuery( const Term& term )
    : Query()
{
    _term = term;

    lcn_term_query_create( &(*this), _term, pool() );
    assertNotNull();
}

NAMESPACE_LCN_END
