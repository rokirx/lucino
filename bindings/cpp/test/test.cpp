#include "test.hpp"

NAMESPACE_LCN_BEGIN

Test::Test()
{
    _failures = 0;
}

void 
Test::incrementFailureCount()
{
    _failures++;
}


TestResult 
Test::result()
{
    return TestResult( testCaseCount(), _failures, _name );
}

TestResult::TestResult( int tests, int failures, const char* className )
{
    _testCount    = tests;
    _failureCount = failures;
    _className    = className;

}

int 
TestResult::testCount() { return _testCount; }

int 
TestResult::failureCount() { return _failureCount; }

void 
TestResult::print( FILE* outFile )
{
    if( 0 == _failureCount )
    {
        fprintf( outFile, "Results for %s:\nAll %d Tests passed\n",
                 (const char*)_className, _testCount );
        return;
    }

    fprintf( outFile, "Results for %s:\n%d Tests, %d successful, %d failures\n",
             (const char*)_className, _testCount, ( _testCount - _failureCount  ), _failureCount );
}

NAMESPACE_LCN_END
