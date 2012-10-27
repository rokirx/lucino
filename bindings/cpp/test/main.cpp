#include "testsuite.hpp"
#include "termquerytest.hpp"
#include "documenttest.hpp"

using namespace lcn;

int main( int argc, char** argv )
{
    Initialize();

    lcn_log_stream = stderr;
    TestSuite suite;

    suite.addTest( new TermQueryTest() );
    suite.addTest( new DocumentTest() );

    return suite.runTests();
}
