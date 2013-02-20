#define _GNU_SOURCE
#include "test_all.h"


static void
test_add_document( CuTest* tc )
{
    apr_pool_t *pool = NULL;
    lcn_document_t *doc;

    LCN_TEST( apr_pool_create( &pool, NULL ));
    LCN_TEST( lcn_document_create( &doc, pool ));
    //LCN_TEST( lcn_doc_helper_setup_doc

    apr_pool_destroy( pool );
}

CuSuite *
make_50_document_writer_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_add_document);

    return s;
}
