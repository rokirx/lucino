#ifndef ANALYZER_HPP
#define ANALYZER_HPP


#include "lcn_analysis.h"
#include "poolobject.hpp"

NAMESPACE_LCN_BEGIN

class Analyzer : public PoolObject<lcn_analyzer_t>
{
public:
    Analyzer();

    enum StdAnalyzer
    {
        SimpleAnalyzer
    };

    static Analyzer fromType( Analyzer::StdAnalyzer );
};

NAMESPACE_LCN_END

#endif /* ANALYZER_HPP */
