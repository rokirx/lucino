#include "documenttest.hpp"
#include "search.hpp"
#include "documentdumpiterator.hpp"

const char* c_str_dump =                                   \
"\t\n \nlcn_document fields = 3\n"                   \
"lcn_field=\"title with \\\"trouble\\\"\"\n"         \
"properties=INDEXED TOKENIZED STORED OMIT_NORMS\n"   \
"lcn_analyzer=lcn_simple_analyzer\n"                 \
"value=\"simple \\\"analyzer\\\"\"\n\n"              \
"lcn_field=\"text\"\n"                               \
"properties=INDEXED TOKENIZED\n"                     \
"lcn_analyzer=lcn_simple_analyzer\n"                 \
"value=\"open\"\n\n"                                 \
"lcn_field=\"data\"\n"                                \
"properties=STORED BINARY\n"                          \
"lcn_analyzer=NONE\n"                                 \
"char bin_value[10]={ 1, 20, 255, 127, 27, 6, 7, 8, 9, 0 }\n\n" \
"\t\n \nlcn_document fields = 2\n"                   \
"lcn_field=\"text\"\n"                               \
"properties=INDEXED TOKENIZED\n"                     \
"lcn_analyzer=lcn_simple_analyzer\n"                 \
"value=\"open\"\n\n"                                 \
"lcn_field=\"data\"\n"                                \
"properties=STORED BINARY\n"                          \
"lcn_analyzer=NONE\n"                                 \
"char bin_value[10]={ 1, 20, 255, 127, 27, 6, 7, 8, 9, 0 }\n\n";


NAMESPACE_LCN_BEGIN
    
DocumentTest::DocumentTest()
{
    LCN_TEST_ADD_CASE( DocumentTest, testDocumentDumpIterator );
}

void 
DocumentTest::setUp()
{
    
}

void 
DocumentTest::tearDown()
{
  
}

void 
DocumentTest::testDocumentDumpIterator()
{
    AnalyzerMap map;
    String dump( c_str_dump );
    Document doc;

    DocumentDumpIterator it( dump, map );

    doc = it.next();

    LCN_ASSERT_EQUAL( doc.fieldCount(), 3 );
    LCN_ASSERT_STR_EQUAL( "title with \"trouble\"", doc.field( 0 ).name() );
    LCN_ASSERT_STR_EQUAL( "text", doc.field( 1 ).name() );
    LCN_ASSERT_STR_EQUAL( "data", doc.field( 2 ).name() );

    doc = it.next();

    LCN_ASSERT_EQUAL( doc.fieldCount(), 2 );
    LCN_ASSERT_STR_EQUAL( "text", doc.field( 0 ).name() );
    LCN_ASSERT_STR_EQUAL( "data", doc.field( 1 ).name() );

}

LCN_TEST_IMPLEMENT( DocumentTest )

NAMESPACE_LCN_END
