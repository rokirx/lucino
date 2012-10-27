#ifndef DOCUMENTDUMPITERATOR_HPP
#define DOCUMENTTUMPITERATOR_HPP

#include "document.hpp"

NAMESPACE_LCN_BEGIN

class DocumentDumpIterator 
    : public PoolObject<lcn_document_dump_iterator_t>
{
public:

    DocumentDumpIterator( const String& input,
                          const AnalyzerMap& map );
    
    Document next();
private:
    String _input;
};

NAMESPACE_LCN_END

#endif /* DOCUMENTDUMPITERATOR */
