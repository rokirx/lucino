#include "booleanquery.hpp"
#include <iostream>


NAMESPACE_LCN_BEGIN

BooleanQuery::BooleanQuery()
    : Query()
{
    lcn_boolean_query_create( &(*this), pool() );
    assertNotNull();
}

void
BooleanQuery::add( const Query& query, Occur occur )
{
    lcn_boolean_query_add( *this, query,
			   occurFromCType( occur ) );
    assertNotNull();
}

BooleanQuery::Occur 
BooleanQuery::occurFromCType( lcn_boolean_clause_occur_t occur )
{
    switch( occur )
    {
    case LCN_BOOLEAN_CLAUSE_SHOULD:
        return Should;

    case LCN_BOOLEAN_CLAUSE_MUST:
        return Must;

    case LCN_BOOLEAN_CLAUSE_MUST_NOT:
        return MustNot;
    }
}

NAMESPACE_LCN_END
