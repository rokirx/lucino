#ifndef TESTSUITE_HPP
#define TESTSUITE_HPP

#include "test.hpp"
#include "list.hpp"

NAMESPACE_LCN_BEGIN

class TestSuite
{
public:
    TestSuite();
    ~TestSuite();
    void addTest( Test* test );

    int runTests();
private:
    List<Test*> _tests;
};

NAMESPACE_LCN_END

#endif

