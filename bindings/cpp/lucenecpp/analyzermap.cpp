#include "analyzermap.hpp"

NAMESPACE_LCN_BEGIN

AnalyzerMap::AnalyzerMap()
    : PoolObject<apr_hash_t>()
{
    lcn_analyzer_map_create( &(*this), pool() );
    
    assertNotNull();
}

void 
AnalyzerMap::add( const Analyzer& analyzer )
{
    _analyzers.push_back( analyzer );

    apr_hash_set( *this, (const void*)lcn_analyzer_type( analyzer ), 
                  APR_HASH_KEY_STRING, 
                  (lcn_analyzer_t*)analyzer );

}

NAMESPACE_LCN_END

