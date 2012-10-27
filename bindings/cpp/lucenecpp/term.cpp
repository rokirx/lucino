#include "term.hpp"

NAMESPACE_LCN_BEGIN

Term::Term()
    : PoolObject<lcn_term_t>()
{
    
}

Term::Term( const String& field, const String& text )
    : PoolObject<lcn_term_t>()
{
    set( field, text );
}

void
Term::set( const String& field, const String& text )
{
    //clear();
    
    lcn_term_create( &(*this),field, text, LCN_TRUE, pool() );

    assertNotNull();
}

String
Term::field() const
{
    return String( lcn_term_field( *this ) );
}

String 
Term::text() const 
{
    return String( lcn_term_text( *this ) );
}

NAMESPACE_LCN_END
