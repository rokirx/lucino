#ifndef TERMQUERYTEST_HPP
#define TERMQUERYTEST_HPP

#include "testsuite.hpp"

NAMESPACE_LCN_BEGIN

class TermQueryTest : public Test
{
    LCN_TEST_DECLARE( TermQueryTest )

public:
    TermQueryTest();
    void setUp();
    void tearDown();
    void run();

    void testQueryToString();
    void testSimpleQuery();
};

NAMESPACE_LCN_END

#endif /* TERMQUERY_TEST */
