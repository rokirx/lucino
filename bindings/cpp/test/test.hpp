#ifndef TEST_HPP
#define TEST_HPP

#include "string.hpp"
#include "list.hpp"


NAMESPACE_LCN_BEGIN

#define LCN_TEST_DECLARE( CLASSNAME )           \
    public:                                     \
    int testCaseCount();                        \
    private:                                    \
    List<void(CLASSNAME::*)()> _testCases;


#define LCN_TEST_IMPLEMENT( CLASSNAME )                 \
    void                                                \
    CLASSNAME::run()                                    \
    {                                                   \
        _name = #CLASSNAME;                             \
        for( int i = 0; i < _testCases.size(); i++ )    \
        {                                               \
            void(CLASSNAME::*oneTestFunc)();           \
                                                        \
            oneTestFunc = _testCases[i];                \
                                                        \
            ((*this).*oneTestFunc)();                   \
        }                                               \
    }                                                   \
    int                                                 \
    CLASSNAME::testCaseCount()                          \
    {                                                   \
        return _testCases.size();                       \
    }

#define LCN_TEST_ADD_CASE( TESTCLASS, TESTCASE )  _testCases.push_back( &TESTCLASS::TESTCASE );

#define LCN_ASSERT( EXPR )                                              \
    {                                                                   \
        if( !( EXPR ) )                                                 \
        {                                                               \
            fprintf( stderr, "Test failed in file %s, line %u: (%s)\n", __FILE__, \
                     __LINE__, #EXPR);                                  \
            incrementFailureCount();                                    \
            return;                                                     \
        }                                                               \
    }

#define LCN_ASSERT_EQUAL( EXPR1, EXPR2 )                                \
    {                                                                   \
        if( ! ( ( EXPR1 ) == ( EXPR2 ) ) )                              \
        {                                                               \
            fprintf( stderr,                                            \
                     "Test failed in file %s, line %u: \"%s\" and \"%s\" are not equal\n", __FILE__, \
                     __LINE__, #EXPR1, #EXPR2 );                        \
            incrementFailureCount();                                    \
            return;                                                     \
        }                                                               \
    }

#define LCN_ASSERT_STR_EQUAL( STR1, STR2 )                              \
    {                                                                   \
        String str1( ( STR1 ) ), str2( ( STR2 ) );                      \
        if( ! ( str1 == str2 ) )                                        \
        {                                                               \
            fprintf( stderr,                                            \
                     "Test failed in file %s, line %u: expected \"%s\"," \
                     "but got \"%s\"\n", __FILE__,                      \
                     __LINE__, (const char*)str1,                       \
                     (const char*)str2 );                               \
            incrementFailureCount();                                    \
            return;                                                     \
        }                                                               \
    }

#define LCN_ASSERT_SUCCESS( EXPR, EXCEPTION_TYPE )                            \
    try {                                                                     \
	EXPR;                                                                 \
    }                                                                         \
    catch( EXCEPTION_TYPE e ) {                                               \
	fprintf( stderr,                                                      \
		 "Test failed in file %s, line %u: \"%s\""                    \
		 " threw an %s : %s\n",                                       \
		 __FILE__, __LINE__, #EXPR, #EXCEPTION_TYPE, e.message() );   \
	incrementFailureCount();                                              \
	return;                                                               \
    }                                                                         \

#define LCN_CHECK_HIT( HITS, N, FIELD, TEXT )                                \
{                                                                            \
    Document doc;                                                            \
                                                                             \
    LCN_ASSERT_SUCCESS(                                                      \
	doc = hits.doc( N ),                                                 \
	IOException );                                                       \
    LCN_ASSERT_STR_EQUAL( doc.get( FIELD ), TEXT );                          \
}

class Test;

class TestResult
{
public:
    int testCount();
    int failureCount();
    void print( FILE* outFile );

private:
    friend class Test;
    TestResult( int tests, int failures, const char* className );

    int _testCount;
    int _failureCount;
    String _className;
};

class Test
{
public:
    Test();
    
    virtual void setUp() = 0;
    virtual void tearDown() = 0;
    virtual void run() = 0;


    TestResult result();

protected:
    virtual int testCaseCount() = 0;
    void incrementFailureCount();
    const char* _name;
private:
    int _failures;
};

NAMESPACE_LCN_END

#endif
