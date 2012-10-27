#ifndef FIELD_HPP
#define FIELD_HPP

#include "lucenecpp.hpp"
#include "poolobject.hpp"
#include "string.hpp"

NAMESPACE_LCN_BEGIN

class Field : public PoolObject<lcn_field_t>
{
public:
    Field();

    String name() const;
    String value() const;

    const unsigned char*
    binaryValue();

    bool binary();
    bool tokenized();
    bool indexed();
    bool omitNorms();
    bool stored();
};

NAMESPACE_LCN_END

#endif /* FIELD_HPP */
