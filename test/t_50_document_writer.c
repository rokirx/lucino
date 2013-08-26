#define _GNU_SOURCE
#include "test_all.h"
#include "index_writer_config.h"


static void
test_add_document( CuTest* tc )
{
    apr_pool_t *pool = NULL;
    lcn_document_t *doc;
    lcn_index_writer_t *index_writer;
    lcn_index_writer_config_t *iwc;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, NULL ));

    /* set up a document */

    LCN_TEST( lcn_document_create( &doc, pool ));
    LCN_TEST( lcn_doc_helper_setup_doc( doc, pool ));


    /* set up an index writer */

    LCN_TEST( lcn_index_writer_config_create( &iwc, pool ) );
    LCN_TEST( lcn_fs_directory_create( &dir, "t_50_document_writer/test_add_document", LCN_TRUE, pool ));
    LCN_TEST( lcn_index_writer_create_by_config( &index_writer, dir, iwc, pool ) );


    /* add document */

    LCN_TEST( lcn_index_writer_add_document( index_writer, doc ));

    //LCN_TEST( lcn_index_writer_commit( index_writer ) );
    //LCN_TEST( lcn_index_writer_close( index_writer ) );

    apr_pool_destroy( pool );
}

CuSuite *
make_50_document_writer_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_add_document);

    return s;
}
