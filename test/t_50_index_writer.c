#define _GNU_SOURCE
#include "test_all.h"
#include "lucene.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "lcn_analysis.h"
#include "index_writer_config.h"


static void
test_index_no_documents( CuTest* tc )
{
    apr_pool_t *pool = NULL;

    do
    {
        lcn_directory_t *dir;
        lcn_index_writer_t *index_writer;
        lcn_index_writer_config_t *iwc;
        //lcn_index_reader_t *index_reader;

        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_directory_create( &dir, pool ) );
        LCN_TEST( lcn_index_writer_config_create( &iwc, pool ) );
        LCN_TEST( lcn_index_writer_create_by_config( &index_writer, dir, iwc, pool ) );
        LCN_TEST( lcn_index_writer_commit( index_writer ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );

        /*
        LCN_TEST( lcn_index_reader_create_by_directory( &index_reader,
                                                        dir,
                                                        LCN_FALSE,
                                                        pool ) );

       LCN_TEST( 0 == lcn_index_reader_max_doc( index_reader ) );
       LCN_TEST( 0 == lcn_index_reader_num_docs( index_reader ) );
       LCN_TEST( lcn_index_reader_close( index_reader ) );
       */
#if 0

      IndexReader reader = DirectoryReader.open(dir);
      assertEquals(0, reader.maxDoc());
      assertEquals(0, reader.numDocs());
      reader.close();

      writer  = new IndexWriter(dir, newIndexWriterConfig( TEST_VERSION_CURRENT, new MockAnalyzer(random())).setOpenMode(OpenMode.APPEND));
      writer.commit();
      writer.close();

      reader = DirectoryReader.open(dir);
      assertEquals(0, reader.maxDoc());
      assertEquals(0, reader.numDocs());
      reader.close();
      dir.close();
 #endif

    }
    while( 0 );

    if( pool != NULL )
    {
        apr_pool_destroy( pool );
    }

}

CuSuite *
make_50_index_writer_suite (void)
{
    CuSuite *s= CuSuiteNew();

#if 1
    SUITE_ADD_TEST(s, test_index_no_documents);
#endif

    return s;
}
