#include "field.hpp"

NAMESPACE_LCN_BEGIN

Field::Field()
  : PoolObject<lcn_field_t>()
{}

String 
Field::name() const
{
    return String( lcn_field_name( *this ) );
}

String 
Field::value() const
{
    return String( lcn_field_value( *this ) );
}

const unsigned char*
Field::binaryValue()
{
    return NULL;
}

bool 
Field::binary()
{
    bool result = (bool)lcn_field_is_binary( *this );
    return result;
}

bool 
Field::tokenized()
{
    return (bool)lcn_field_is_tokenized( *this );
}

bool 
Field::indexed()
{
    return (bool)lcn_field_is_indexed( *this );
}

bool 
Field::stored()
{
    return (bool)lcn_field_is_stored( *this );
}

bool 
Field::omitNorms()
{
    return (bool)lcn_field_omit_norms( *this );
}

NAMESPACE_LCN_END
