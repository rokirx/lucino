#include "termquerytest.hpp"
#include "termquery.hpp"
#include "search.hpp"

NAMESPACE_LCN_BEGIN
    
TermQueryTest::TermQueryTest()
{
    LCN_TEST_ADD_CASE( TermQueryTest, testQueryToString );
    LCN_TEST_ADD_CASE( TermQueryTest, testSimpleQuery );
}

void 
TermQueryTest::setUp()
{
    
}

void 
TermQueryTest::tearDown()
{
  
}

void
TermQueryTest::testQueryToString()
{
    TermQuery query( Term( "field", "text" ) );
    LCN_ASSERT_STR_EQUAL( query.toString( "" ), "field:text" );
}

void
TermQueryTest::testSimpleQuery()
{
    IndexSearcher searcher;

    LCN_ASSERT_SUCCESS( searcher.open( "../../../../build/test/test_index_1" ),
			IOException );
 
    Hits hits;
    LCN_ASSERT_SUCCESS(
	hits = searcher.search( TermQuery( Term( "text", "can" ) ) ),
	IOException );
    
    LCN_ASSERT_EQUAL( 9, hits.length() );

    LCN_CHECK_HIT( hits, 0, "id", "KW21" );
    LCN_CHECK_HIT( hits, 1, "id", "KW67" );
    LCN_CHECK_HIT( hits, 2, "id", "KW70" );
    LCN_CHECK_HIT( hits, 3, "id", "KW105" );
    LCN_CHECK_HIT( hits, 4, "id", "KW172" );
    LCN_CHECK_HIT( hits, 5, "id", "KW299" );
    LCN_CHECK_HIT( hits, 6, "id", "KW322" );
    LCN_CHECK_HIT( hits, 7, "id", "KW45" );
    LCN_CHECK_HIT( hits, 8, "id", "KW130" );
    
    LCN_ASSERT_SUCCESS( searcher.close(),
			IOException );
}

LCN_TEST_IMPLEMENT( TermQueryTest )

NAMESPACE_LCN_END
