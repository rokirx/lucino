#ifndef DOCUMENTTEST_HPP
#define DOCUMENTTEST_HPP

#include "testsuite.hpp"

NAMESPACE_LCN_BEGIN

class DocumentTest : public Test
{
    LCN_TEST_DECLARE( DocumentTest )

public:
    DocumentTest();
    void setUp();
    void tearDown();
    void run();

    void testDocumentDumpIterator();
};

NAMESPACE_LCN_END

#endif /* DOCUMENT_TEST */
