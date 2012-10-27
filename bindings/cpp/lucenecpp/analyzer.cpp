#include "analyzer.hpp"

NAMESPACE_LCN_BEGIN

Analyzer::Analyzer()
  : PoolObject<lcn_analyzer_t>()
{

}
Analyzer 
Analyzer::fromType( Analyzer::StdAnalyzer az )
{
    Analyzer result;

    switch( az )
    {
    case SimpleAnalyzer:
        lcn_simple_analyzer_create( &result, result.pool() );
        break;
    }

    result.assertNotNull();

    return result;
}

NAMESPACE_LCN_END
