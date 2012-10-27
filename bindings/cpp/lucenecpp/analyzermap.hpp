#ifndef ANALYZERMAP_HPP
#define ANALYZERMAP_HPP


#include "poolobject.hpp"
#include "analyzer.hpp"
#include "string.hpp"
#include "list.hpp"

NAMESPACE_LCN_BEGIN

class AnalyzerMap : public PoolObject<apr_hash_t>
{
public:
    AnalyzerMap();
    void add( const Analyzer& analyzer );

private:
    List<Analyzer> _analyzers;
};

NAMESPACE_LCN_END

#endif /* ANALYZERMAP_HPP */
