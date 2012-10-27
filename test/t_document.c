#include "test_all.h"
#include "lcn_analysis.h"
#include "lcn_index.h"

const char* dump =                                                      \
"\t\n \nlcn_document fields = 3\n"                                      \
"lcn_field=\"title with \\\"trouble\\\"\"\n"                            \
"properties=INDEXED TOKENIZED STORED OMIT_NORMS\n"                      \
"lcn_analyzer=lcn_simple_analyzer\n"                                    \
"value=\"simple \\\"analyzer\\\"\"\n\n"                                 \
"lcn_field=\"text\"\n"                                                  \
"properties=INDEXED TOKENIZED\n"                                        \
"lcn_analyzer=lcn_simple_analyzer\n"                                    \
"value=\"open\"\n\n"                                                    \
"lcn_field=\"data\"\n"                                                  \
"properties=STORED BINARY\n"                                            \
"lcn_analyzer=NONE\n"                                                   \
"char bin_value[10]={ 1, 20, 255, 127, 27, 6, 7, 8, 9, 0 }\n\n"         \
"\t\n \nlcn_document fields = 2\n"                                      \
"lcn_field=\"text\"\n"                                                  \
"properties=INDEXED TOKENIZED\n"                                        \
"lcn_analyzer=lcn_simple_analyzer\n"                                    \
"value=\"open\"\n\n"                                                    \
"lcn_field=\"data\"\n"                                                  \
"properties=STORED BINARY\n"                                            \
"lcn_analyzer=NONE\n"                                                   \
"char bin_value[10]={ 1, 20, 255, 127, 27, 6, 7, 8, 9, 0 }\n\n";


const char *bin_val  = "this text will be stored as a byte array in the index";
const char *bin_val2 = "this text will be also stored as a byte array in the index";


static void
test_create_from_dump( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_document_dump_iterator_t* it;
    lcn_document_t* doc;
    apr_hash_t* analyzers;
    lcn_field_t* field;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_analyzer_map_create( &analyzers, pool ) );
    LCN_TEST( lcn_document_dump_iterator_create( &it, dump, analyzers, pool ) );
    LCN_TEST( lcn_document_dump_iterator_next( it, &doc, pool ) );

    CuAssertIntEquals( tc, 3, lcn_list_size( lcn_document_fields( doc ) ) );

    field = lcn_list_get( lcn_document_fields( doc ), 0 );

    CuAssertStrEquals( tc, "title with \"trouble\"", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "simple \"analyzer\"", lcn_field_value( field ) );

    field = lcn_list_get( lcn_document_fields( doc ), 1 );

    CuAssertStrEquals( tc, "text", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "open", lcn_field_value( field ) );

    LCN_TEST( lcn_document_dump_iterator_next( it, &doc, pool ) );

    field = lcn_list_get( lcn_document_fields(
                              doc ), 0 );
    CuAssertStrEquals( tc, "text", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "open", lcn_field_value( field ) );

    field = lcn_list_get( lcn_document_fields( doc ), 1 );
    CuAssertStrEquals( tc, "data", lcn_field_name( field ) );

    LCN_TEST_STATUS( lcn_document_dump_iterator_next( it, &doc, pool ), LCN_ERR_ITERATOR_NO_NEXT );

    apr_pool_destroy( pool );
}

static void
test_binary_field( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_document_t* doc;
    lcn_field_type_t ft;
    lcn_field_t *string_field = NULL;
    lcn_field_t *binary_field, *binary_field2;
    lcn_list_t *list;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_document_create ( &doc, pool ));
    LCN_TEST( lcn_field_type_init( &ft ));

    LCN_TEST( lcn_field_type_set_stored( &ft, LCN_TRUE ));

    CuAssertTrue( tc, lcn_field_type_is_stored( ft ));

    LCN_TEST( lcn_field_create_ft( &string_field,
                                   "string",
                                   bin_val,
                                   ft,
                                   pool ));

    CuAssertPtrNotNull( tc, string_field );
    CuAssertTrue( tc, lcn_field_is_stored( string_field ));

#if 0
    LCN_TEST( lcn_field_create_binary_ft( &binary_field,
                                          "binary",
                                          bin_val,
                                          strlen(bin_val),
                                          ft,
                                          pool ));

    LCN_TEST( lcn_field_create_binary_ft( &binary_field2,
                                          "binary",
                                          bin_val2,
                                          strlen(bin_val2),
                                          ft,
                                          pool ));
#endif
    //LCN_TEST( lcn_document_add_field( doc, string_field, pool ));

    //LCN_TEST( lcn_document_add_field( doc, binary_field, pool ));
    //list = lcn_document_fields( doc );

    //CuAssertIntEquals( tc, 2, lcn_list_size( list ));

    apr_pool_destroy( pool );
}


#if 0
  public void testBinaryField() throws Exception {

// -->

    assertTrue(binaryFld.binaryValue() != null);
    assertTrue(binaryFld.fieldType().stored());
    assertFalse(binaryFld.fieldType().indexed());
    assertFalse(binaryFld.fieldType().tokenized());

    String binaryTest = doc.getBinaryValue("binary").utf8ToString();
    assertTrue(binaryTest.equals(binaryVal));

    String stringTest = doc.get("string");
    assertTrue(binaryTest.equals(stringTest));

    doc.add(binaryFld2);

    assertEquals(3, doc.getFields().size());

    BytesRef[] binaryTests = doc.getBinaryValues("binary");

    assertEquals(2, binaryTests.length);

    binaryTest = binaryTests[0].utf8ToString();
    String binaryTest2 = binaryTests[1].utf8ToString();

    assertFalse(binaryTest.equals(binaryTest2));

    assertTrue(binaryTest.equals(binaryVal));
    assertTrue(binaryTest2.equals(binaryVal2));

    doc.removeField("string");
    assertEquals(2, doc.getFields().size());

    doc.removeFields("binary");
    assertEquals(0, doc.getFields().size());
  }
#endif


CuSuite*
make_document_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_create_from_dump );
    SUITE_ADD_TEST( s, test_binary_field );

    return s;
}
