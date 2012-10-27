#include "documentdumpiterator.hpp"
#include "lcn_analysis.h"

NAMESPACE_LCN_BEGIN

DocumentDumpIterator::DocumentDumpIterator( const String& input,
                                            const AnalyzerMap& map )
    : PoolObject<lcn_document_dump_iterator_t>()
{
    _input = input;
    lcn_document_dump_iterator_create( &(*this), _input, map, pool() );
}

Document
DocumentDumpIterator::next()
{
    apr_status_t s;
    Document result;

    if( ( s = lcn_document_dump_iterator_next( *this, &result, 
                                               result.pool() ) ) )
    {
        result.clear();
    }

    return result;
}

NAMESPACE_LCN_END
