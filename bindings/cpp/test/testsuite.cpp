#include "testsuite.hpp"

NAMESPACE_LCN_BEGIN

TestSuite::TestSuite()
{
    
}

TestSuite::~TestSuite()
{
    for( int i = 0; i < _tests.size(); i++ )
    {
        delete _tests[i];
    }
}

void 
TestSuite::addTest( Test* test )
{
    _tests.push_back( test );
}

int
TestSuite::runTests()
{
    FILE* outFile = stdout;
    int tests = 0;
    int failures = 0;

    for( int i = 0; i < _tests.size(); i++ )
    {
        _tests[i]->setUp();
        _tests[i]->run();
        _tests[i]->tearDown();

        TestResult result = _tests[i]->result();

        result.print( outFile );

        tests    += result.testCount();
        failures += result.failureCount();

        fprintf( outFile, "\n" );
    }
    
    fprintf( outFile, "Ran %d tests, ", tests );

    if( failures == 0 )
    {
        fprintf( outFile, "all successful\n\n" );
        return 0;
    }

    fprintf( outFile, "%d failures...\n\n", failures );
    return -1;
}


NAMESPACE_LCN_END
