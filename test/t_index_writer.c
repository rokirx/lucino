#define _GNU_SOURCE
#include "test_all.h"
#include "lucene.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "lcn_analysis.h"
#include "index_writer_config.h"

char *test_ids[] = { "162879",  "162880",  "162881",  "162882",  "162883",
                     "162884",  "162885",  "162886",  "162887",  "162888",
                     "162889",  "163007",  "172251",  "197130",  "197131",
                     "197132",  "197133",  "197134",  "197135",  "197136",
                     "197137",  "197138",  "197139",  "197140",  "197141",
                     "197142",  "197143",  "197144",  "197145",  "197146",
                     "197147",  "197148",  "197149",  "197150",  "197151",
                     "197986",  "226965",  "227256",  "227433",  "227434",
                     "254412",  "285812",  "285813",  "285814",  "285815",
                     "285816",  "285817",  "285818",  "285819",  "285820",
                     "285821",  "285822",  "285823",  "285824",  "285825",
                     "285826",  "285827",  "285828",  "285829",  "285830",
                     "285831",  "285832",  "285833",  "285834",  "285835",
                     "285836",  "285837",  "285838",  "285839",  "285840",
                     "285841",  "285842",  "285843",  "285844",  "285845",
                     "285846",  "285847",  "285848",  "162877",  "162878",  0 };

static void
add_doc( CuTest* tc,
         lcn_index_writer_t *index_writer,
         apr_pool_t *pool )
{
    lcn_document_t *doc;
    lcn_analyzer_t *analyzer;
    apr_pool_t *p;
    lcn_field_t *field;

    LCN_TEST( apr_pool_create( &p, pool ));
    LCN_TEST( lcn_document_create( &doc, p ) );
    LCN_TEST( lcn_field_create( &field, "content", "aaa",
                                LCN_FIELD_INDEXED | LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, p ) );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, p ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( doc, field, p ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, doc ) );
}

static void
test_doc_count(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_ram_directory_create( &dir, pool ) );

#if 0
    /* TODO: writeLockTimeOut */
    IndexWriter.setDefaultWriteLockTimeout(2000);
    assertEquals(2000, IndexWriter.getDefaultWriteLockTimeout());
#endif
#if 0
    LCN_TEST( lcn_index_writer_create_by_directory( &index_writer,
                                                    dir,
                                                    LCN_TRUE, /* create */
                                                    pool ));

    // IndexWriter.setDefaultWriteLockTimeout(1000);

    for( i = 0; i < 100; i++ )
    {
        add_doc( tc, index_writer, pool );
    }

    //CuAssertIntEquals( tc, 100, lcn_index_writer_doc_count( index_writer ));

#endif
#if 0
        assertEquals(100, writer.docCount());
        writer.close();

        // delete 40 documents
        reader = IndexReader.open(dir);
        for (i = 0; i < 40; i++) {
            reader.deleteDocument(i);
        }
        reader.close();

        // test doc count before segments are merged/index is optimized
        writer = new IndexWriter(dir, new WhitespaceAnalyzer(), IndexWriter.MaxFieldLength.LIMITED);
        assertEquals(100, writer.docCount());
        writer.close();

        reader = IndexReader.open(dir);
        assertEquals(100, reader.maxDoc());
        assertEquals(60, reader.numDocs());
        reader.close();

        // optimize the index and check that the new doc count is correct
        writer = new IndexWriter(dir, true, new WhitespaceAnalyzer(), IndexWriter.MaxFieldLength.LIMITED);
        assertEquals(100, writer.maxDoc());
        assertEquals(60, writer.numDocs());
        writer.optimize();
        assertEquals(60, writer.maxDoc());
        assertEquals(60, writer.numDocs());
        writer.close();

        // check that the index reader gives the same numbers.
        reader = IndexReader.open(dir);
        assertEquals(60, reader.maxDoc());
        assertEquals(60, reader.numDocs());
        reader.close();

        // make sure opening a new index for create over
        // this existing one works correctly:
        writer = new IndexWriter(dir, new WhitespaceAnalyzer(), true, IndexWriter.MaxFieldLength.LIMITED);
        assertEquals(0, writer.maxDoc());
        assertEquals(0, writer.numDocs());
        writer.close();


#endif





#if 0
    delete_files( tc, "test_index_writer" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_document_create( &document, pool ) );
    LCN_TEST( lcn_field_create( &field, "text", "open source", LCN_FIELD_STORED, LCN_FIELD_VALUE_COPY, pool ) );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_01", "test_index_writer" );
    delete_files( tc, "test_index_writer" );
#endif
    apr_pool_destroy( pool );
}

static void
test_creation(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_document_create( &document, pool ) );
    LCN_TEST( lcn_field_create( &field, "text", "open source", LCN_FIELD_STORED, LCN_FIELD_VALUE_COPY, pool ) );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_01", "test_index_writer" );
    delete_files( tc, "test_index_writer" );
    apr_pool_destroy( pool );
}

static void
test_adding_empty_document(CuTest* tc)
{
    apr_pool_t *pool;

    delete_files( tc, "test_index_writer" );
    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    delete_files( tc, "test_index_writer" );
    apr_pool_destroy( pool );
}



static void
test_merge_terms(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open source",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_02", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

static void
test_merge_terms_2(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open source",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

    LCN_TEST( lcn_document_create( &document, pool ) );
    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open source",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_03", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

static void
test_indexing_4(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_04", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

static void
test_indexing_5(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_t *field1;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );

    LCN_TEST( lcn_field_create( &field1,
                                "titel",
                                "simple analyzer",
                                LCN_FIELD_INDEXED   |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field1, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_document_add_field( document, field1, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_05", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}


static void
test_indexing_6(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_t *field1;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );

    LCN_TEST( lcn_field_create( &field1,
                                "titel",
                                "simple analyzer",
                                LCN_FIELD_INDEXED   |
                                LCN_FIELD_TOKENIZED |
                                LCN_FIELD_OMIT_NORMS |
                                LCN_FIELD_STORED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field1, analyzer );

    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_document_add_field( document, field1, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_06", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

static void
test_indexing_7(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_t *field1;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );

    LCN_TEST( lcn_field_create_binary( &field1,
                                       "titel",
                                       "\366l ma\337",
                                       LCN_FIELD_VALUE_COPY,
                                       strlen("\366l ma\337"),
                                       pool ) );

    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_document_add_field( document, field1, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_07", "test_index_writer" );

    /* check reading of the field binary field */

    {
        lcn_index_reader_t *reader;
        lcn_document_t *doc;
        char *buf;
        unsigned int len;

        LCN_TEST( lcn_index_reader_create_by_path( &reader,
                                                   "test_index_writer",
                                                   pool ));

        LCN_TEST( lcn_index_reader_document( reader,
                                             &doc,
                                             0,
                                             pool ));

        LCN_TEST( lcn_document_get_binary_field_value( doc,
                                                 "titel",
                                                 &buf,
                                                 &len,
                                                 pool ));
        CuAssertIntEquals( tc, 6, len );
        CuAssertIntEquals( tc, 0, strncmp( buf, "\366l ma\337", 6 ));
    }

    delete_files( tc, "test_index_writer" );
    apr_pool_destroy( pool );
}

static void
test_indexing_8(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_t *field1;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create_binary( &field,
                                       "text",
                                       "\366a\337",
                                       LCN_FIELD_VALUE_COPY,
                                       strlen("\366a\337"),
                                       pool ) );


    LCN_TEST( lcn_field_create( &field1,
                                "titel",
                                "first",
                                LCN_FIELD_INDEXED,
                                LCN_FIELD_VALUE_COPY, pool ) );

    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_document_add_field( document, field1, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

    compare_directories(tc, "index_writer/index_08", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}


static void
test_indexing_8a(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "a b c",
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY,
                                pool ) );

    lcn_field_set_analyzer( field, analyzer );

    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );

#if 0
    compare_directories(tc, "index_writer/index_08", "test_index_writer" );
    delete_files( tc, "test_index_writer" );
#endif

    apr_pool_destroy( pool );
}


static void add_document( CuTest* tc,
                          lcn_index_writer_t *index_writer,
                          const char *text_val,
                          const char *title_val,
                          apr_pool_t *pool )
{
    apr_pool_t *p;
    lcn_document_t *document;
    lcn_field_t *field, *field1;

    LCN_TEST( apr_pool_create( &p, pool ) );
    LCN_TEST( lcn_document_create( &document, p ) );
    LCN_TEST( lcn_field_create_binary( &field,
                                       "text",
                                       text_val,
                                       LCN_FIELD_VALUE_COPY,
                                       strlen(text_val),
                                       p ) );


    LCN_TEST( lcn_field_create( &field1,
                                "titel",
                                title_val,
                                LCN_FIELD_INDEXED | LCN_FIELD_OMIT_NORMS,
                                LCN_FIELD_VALUE_COPY, p ) );

    LCN_TEST( lcn_document_add_field( document, field, p ) );
    LCN_TEST( lcn_document_add_field( document, field1, p ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

    apr_pool_destroy( p );
}

static void
test_indexing_9(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    add_document( tc, index_writer, "\366a\337", "first", pool );
    add_document( tc, index_writer, "123\326", "second", pool );

    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_09", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

#if 0
static void
test_indexing_10(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );

    lcn_index_writer_set_max_buffered_docs( index_writer, 3 );
    lcn_index_writer_set_merge_factor( index_writer, 4 );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    add_document( tc, index_writer, "\366a\337", "first", pool );
    add_document( tc, index_writer, "123\326", "second", pool );
    add_document( tc, index_writer, "$%&", "third", pool );
    add_document( tc, index_writer, "\3661\337", "4th", pool );
    add_document( tc, index_writer, "133\326", "5th", pool );
    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_09", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}
#endif

static void
test_indexing_11(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    add_document( tc, index_writer, "\366a\337", "first", pool );
    add_document( tc, index_writer, "123\326", "second", pool );

    LCN_TEST( lcn_index_writer_close( index_writer ) );


    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_09", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

/**
 * TODO: Check Test is not running
 */
static void
TestCuStress(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_analyzer_t *analyzer;
    unsigned int i;
    apr_pool_t *p;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );

    //lcn_index_writer_set_max_buffered_docs( index_writer, 4 );
    //lcn_index_writer_set_merge_factor( index_writer, 3 );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );


    apr_pool_create( &p, pool );

    for( i = 0; i < 64000; i++ )
    {
        char buf[10];
        char dec[20];
        lcn_itoa36( i, buf );
        sprintf( dec, "%d", i );
        add_document( tc, index_writer, buf, dec, p );
        apr_pool_clear( p );
    }

    LCN_TEST( lcn_index_writer_close( index_writer ) );
    //compare_directories(tc, "index_writer/index_09", "test_index_writer" );
    //delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}


static void
test_empty_field(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_t *field1;
    lcn_analyzer_t *analyzer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "open",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field, analyzer );

    LCN_TEST( lcn_field_create( &field1,
                                "titel",
                                "  ;-)",
                                LCN_FIELD_INDEXED   |
                                LCN_FIELD_TOKENIZED |
                                LCN_FIELD_OMIT_NORMS |
                                LCN_FIELD_STORED,
                                LCN_FIELD_VALUE_COPY, pool ) );
    lcn_field_set_analyzer( field1, analyzer );

    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_document_add_field( document, field1, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

    LCN_TEST( lcn_index_writer_close( index_writer ) );
    compare_directories(tc, "index_writer/index_empty_field", "test_index_writer" );
    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}

static void
test_compound_file(CuTest* tc)
{
#if 0
    lucene_FSDirectory *write_dir;
    lucene_create_FSDirectory( &write_dir, "./cfs_index/");
    lucene_IndexWriter *w = lucene_create_IndexWriter( write_dir, FALSE );
    CuAssertPtrNotNull( tc, w );
    //IndexWriter *i_writer = (IndexWriter*) w->st_index_writer;

    w->set_use_compound_file( w, TRUE );
    CuAssertIntEquals(tc, LUCENE_OK, w->optimize( w ) );

    w->free( w );
    write_dir->free( write_dir );
#endif
}


#if 0
void test_indexing(CuTest* tc)
{
    lucene_Analyzer *an = lucene_create_SimpleAnalyzer();

    char buf[100];
    char vtext[100];
    char dir_name[100];

    int doc_number;
    int test_number;

    for (test_number = 329; test_number < 330; test_number++ )
    {

        lucene_FSDirectory *dir;
        lucene_IndexWriter *w;

        sprintf(dir_name, "%s%d/", "test_dir_", test_number );

        //lucene_create_FSDirectory( &dir, dir_name );
        w = lucene_create_IndexWriter( dir, 1 );

        for ( doc_number = 0; doc_number < test_number; doc_number++)
        {
            lucene_Document *doc = lucene_create_Document();
            lucene_Field f;
            lucene_Field v;

            sprintf(buf, "%d", doc_number );
            strcpy( vtext, data[ doc_number ] );
            lucene_init_Field( &f, "xid", buf, LUCENE_FIELD_STORED | LUCENE_FIELD_UNWEIGHTED );
            lucene_init_StoredTokenizedIndexedWeightedField(&v, "volltext", vtext, an );
            v.set_binary_len( &v, strlen(vtext) );

            doc->add( doc, &f );
            doc->add( doc, &v );

            w->add_document( w, doc );

            doc->free( doc );
        }

        w->optimize( w );

        w->free( w );
        dir->free( dir );
    }

    an->free( an );
}
#endif

#if 0
void test_indexing_1(CuTest* tc)
{
    FILE *fh = fopen("/home/rk/OpenSource/list", "r" );
    char *line = NULL;// = (char *) malloc(1000);
    int i = 0;

    lucene_Analyzer *an = lucene_create_GermanAnalyzer();

    //char buf[100];
    char vtext[100];
    char dir_name[] = "/home/rk/TESTIND_C/";

    int doc_number;
    //int test_number;

    char *fbuf = (char *) malloc(4000000);
    char *cbuf = (char *) malloc(4000000);

    lcn_directory_t *dir;
    apr_pool_t *pool;
    apr_pool_create( &pool, NULL );
    lcn_fs_directory_create( &dir, dir_name, LCN_FALSE, pool );
    IndexWriter *w = create_IndexWriter( dir, 1 );

    for ( doc_number = 0; doc_number < 36000; doc_number++) //36000
    {
        Document *doc = create_Document();
        off_t size;
        Field *f;
        Field* v;
        Field *h ;
        unsigned int dest_len ;
        //int res;

        if ( doc_number % 100 == 0 )
        {
            printf("%d\n", doc_number );
        }

        getline( &line, &i, fh);
        line[ strlen(line) -1] = 0;
        //printf("1 -> %s\n", line);

        strcpy( vtext, line );
#if 0
        os_open_read_only( vtext, &file );

        os_file_size( &file, &size );
        if ( LUCENE_OK != os_read( &file, fbuf, size ) )
        {
            //printf("MISTSSSSSSSSSSSSS\n");
            exit(0);
        }

        for ( i = 0; i < size; i++ )
        {
            //printf("%c", fbuf[i] );
        }
        fbuf[size] = '\0';
        os_close( &file );
        //printf("-> %s\n", vtext);
#endif
        f = create_Field("xid", vtext, 1, 0, 0, 0, 0, 0 );
        v = create_Field("volltext", fbuf, 0, 1, 1, 1, 0, 0 );

        dest_len = 4000000 - 4;
        //res = BZ2_bzBuffToBuffCompress( cbuf + 4, &dest_len, fbuf, size, 9, 0, 30 );

        cbuf[0] = (char) (unsigned char) (dest_len >> 24);
        cbuf[1] = (char) (unsigned char) (dest_len >> 16);
        cbuf[2] = (char) (unsigned char) (dest_len >>  8);
        cbuf[3] = (char) (unsigned char)  dest_len;

        h= create_Field("html", cbuf, 1, 0, 0, 0, 1, 0 );

        h->binary_len = dest_len + 4;

        doc->add( doc, f );
        doc->add( doc, v );
        doc->add( doc, h );

        w->add_document( w, doc );

        doc->free( doc );
    }

    free( fbuf );
    free( cbuf );
    free( line );
    w->optimize( w );
    w->free( w );

    an->free( an );
}
#endif

#if 0
void TestCuDocumentWriter(CuTest* tc)
{
    lcn_directory_t *dir;
    lucene_Analyzer *an = lucene_create_SimpleAnalyzer();
    Similarity *sim = create_Similarity();
    DocumentWriter *w;
    Document *doc;
    char *buf, *vtext, *s, *seg_name;
    Field *f, *v;
    FieldInfos *f_test;
    lcn_index_input_t *in;
    int len;
    unsigned int ui;

    lcn_ram_directory_create( &dir, NULL );

    CuAssertTrue(tc, an != NULL);

    w = create_DocumentWriter( dir, sim, 100 );

    doc = create_Document();
    buf = (char *) malloc( sizeof(char) * 4 );
    strcpy( buf, "123" );
    vtext = (char *) malloc( sizeof(char) * 14);
    strcpy( vtext, "a b c a b d a");
    f = create_Field( "xid", buf, 1, 0, 0, 0, 0, 0 );
    v = create_Field( "volltext", vtext, 0, 1, 1, 1, 0, 0 );
    doc->add( doc, f );
    doc->add( doc, v );

    seg_name = (char *) malloc( sizeof(char) * 4);
    strcpy( seg_name, "_k1" );

    w->add_document( w, seg_name, doc );

    CuAssertTrue(tc, w->field_infos->size == 3 );
    CuAssertTrue(tc, strcmp( "volltext", w->field_infos->first_info->next->name ) == 0);
    CuAssertTrue(tc, strcmp( "xid", w->field_infos->first_info->next->next->name ) == 0);

    f_test = dir->read_field_infos( dir, "_k1" );
    CuAssertTrue(tc, f_test->size == 3 );
    CuAssertTrue(tc, strcmp( "volltext", f_test->first_info->next->name ) == 0 );
    CuAssertTrue(tc, strcmp( "xid", f_test->first_info->next->next->name ) == 0);
    f_test->free( f_test );

    dir->open_file( dir, &in, "_k1.fdt" );

    in->read_vint( in, &ui );
    CuAssertIntEquals(tc, ui, 1 ); /* stored count                             */
    in->read_vint( in, &ui );
    CuAssertIntEquals(tc, ui, 2 ); /* field number                             */
    in->read_vint( in, &ui );
    CuAssertIntEquals(tc, ui, 0 ); /* field bits (properties) 0: not tokenized */
    s = NULL;
    in->read_string( in, &s, &len, NULL );
    CuAssertTrue(tc, 3 == len);
    CuAssertTrue(tc, strcmp( "123", s ) == 0 );
    free( s );

    free( seg_name );
    doc->free( doc );
    an->free( an );
    w->free( w );
    free_Similarity( sim );
}
#endif

#if 0
void TestCuSegName(CuTest* tc)
{
    char buf[8];

    IndexWriter *w = create_IndexWriter( write_dir, 1 );
    CuAssertPtrNotNull(tc, w );

    buf[0] = '_';

    w->new_seg_name( 0, buf + 1 );
    CuAssertStrEquals(tc, "_0", buf );

    w->new_seg_name( 1, buf + 1 );
    CuAssertStrEquals(tc, "_1", buf );

    w->new_seg_name( 31, buf + 1 );
    CuAssertStrEquals(tc, "_v", buf );

    w->new_seg_name( 32, buf + 1 );
    CuAssertStrEquals(tc, "_10", buf );

    w->new_seg_name( 70, buf + 1 );
    CuAssertStrEquals(tc, "_26", buf );

    w->new_seg_name( 32 * 32, buf + 1 );
    CuAssertStrEquals(tc, "_100", buf );

    w->new_seg_name( 2 * 32 * 32 + 7 * 32 + 5, buf + 1 );
    CuAssertStrEquals(tc, "_275", buf );

    w->new_seg_name( 4 * 32 * 32 * 32 + 2 * 32 * 32 + 7 * 32 + 5, buf + 1 );
    CuAssertStrEquals(tc, "_4275", buf );

    w->new_seg_name( 10 * 32 * 32 * 32 * 32 + 4 * 32 * 32 * 32 + 2 * 32 * 32 + 7 * 32 + 5, buf + 1 );
    CuAssertStrEquals(tc, "_a4275", buf );

    w->new_seg_name( 11 * 32 * 32 * 32 * 32 * 32 + 4 * 32 * 32 * 32 + 2 * 32 * 32 + 7 * 32 + 5, buf + 1 );
    CuAssertStrEquals(tc, "_b04275", buf );

    w->new_seg_name( (unsigned int) (0xc0000000), buf + 1);
    CuAssertStrEquals(tc, "_3000000", buf );

    w->free( w );
}
#endif

static void
test_deleting_empty_index(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_empty_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );
    LCN_TEST( lcn_index_writer_delete_if_empty( index_writer ) );

    apr_pool_destroy( pool );
}

#if 0
static void
test_add_indexes( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_list_t *list;
    lcn_directory_t *dir;
#if 0

product_a1867_Thread[Thread-3,5,main]
product_a1867_Thread[Thread-4,5,main]
product_a1867_Thread[Thread-5,5,main]
product_a1867_Thread[Thread-6,5,main]
#endif


    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "/home/rk/valtest/product_a1867/product_a1867_Thread[Thread-2,5,main]", LCN_FALSE, pool ) );
    LCN_TEST( lcn_list_create( &list, 10, pool ));

    LCN_TEST( lcn_fs_directory_create( &dir, "/home/rk/valtest/product_a1867/product_a1867_Thread[Thread-3,5,main]", LCN_FALSE, pool ) );
    LCN_TEST( lcn_list_add( list, dir ));
    LCN_TEST( lcn_fs_directory_create( &dir, "/home/rk/valtest/product_a1867/product_a1867_Thread[Thread-4,5,main]", LCN_FALSE, pool ) );
    LCN_TEST( lcn_list_add( list, dir ));
    LCN_TEST( lcn_fs_directory_create( &dir, "/home/rk/valtest/product_a1867/product_a1867_Thread[Thread-5,5,main]", LCN_FALSE, pool ) );
    LCN_TEST( lcn_list_add( list, dir ));
    LCN_TEST( lcn_fs_directory_create( &dir, "/home/rk/valtest/product_a1867/product_a1867_Thread[Thread-6,5,main]", LCN_FALSE, pool ) );
    LCN_TEST( lcn_list_add( list, dir ));

    LCN_TEST( lcn_index_writer_add_indexes( index_writer, list ));

    LCN_TEST( lcn_index_writer_close( index_writer ));
    apr_pool_destroy( pool );
}
#endif

CuSuite *make_index_writer_suite (void)
{
    CuSuite *s= CuSuiteNew();

#if 1
    SUITE_ADD_TEST(s, test_doc_count             );
    SUITE_ADD_TEST(s, test_adding_empty_document );
    SUITE_ADD_TEST(s, test_creation              );
    SUITE_ADD_TEST(s, test_deleting_empty_index  );
    SUITE_ADD_TEST(s, test_compound_file         );
    SUITE_ADD_TEST(s, test_indexing_4            );
    SUITE_ADD_TEST(s, test_indexing_5            );
    SUITE_ADD_TEST(s, test_indexing_6            );
    SUITE_ADD_TEST(s, test_indexing_7            );
    SUITE_ADD_TEST(s, test_indexing_8            );
    SUITE_ADD_TEST(s, test_indexing_8a           );
    SUITE_ADD_TEST(s, test_indexing_9            );
    SUITE_ADD_TEST(s, test_merge_terms           );
    SUITE_ADD_TEST(s, test_merge_terms_2         );
    SUITE_ADD_TEST(s, test_empty_field           );
    SUITE_ADD_TEST(s, test_indexing_11           );
#endif

    //SUITE_ADD_TEST(s, test_add_indexes           );
    //SUITE_ADD_TEST(s, test_indexing_10           );
    //SUITE_ADD_TEST(s,TestCuStress);

    return s;
}
