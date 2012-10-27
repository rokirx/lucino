#ifndef HASH_HPP
#define HASH_HPP


#include "apr_hashes.h"
#include "poolobject.hpp"
#include "string.hpp"

NAMESPACE_LCN_BEGIN

class AnalyzerMap : private PoolObject<apr_hash_t>
{
public:
    AnalyzerMap();

    void addAnalyzer( const String& name, 
                      lcn_analyzer_t* analyzer );
};

NAMESPACE_LCN_END

#endif /* HASH_HPP */
