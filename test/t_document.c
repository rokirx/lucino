#include "test_all.h"
#include "lcn_analysis.h"
#include "lcn_index.h"

const char* dump =                                                      \
"\t\n \nlcn_document fields = 4\n"                                      \
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
"lcn_field=\"unicodefield\"\n"						\
"properties=STORED INDEXED TOKENIZED\n"						\
"lcn_analyzer=lcn_simple_analyzer\n"					\
"value=\"schöne grüße передача\"\n\n"							\
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

/*
struct iw
 ...
lcn_iwc_t iwc;

int f( lcn_iwc_t *iwc):
...
iw->iwc = *iwc;
*/

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

    CuAssertIntEquals( tc, 4, lcn_list_size( lcn_document_get_fields( doc ) ) );

    field = lcn_list_get( lcn_document_get_fields( doc ), 0 );

    CuAssertStrEquals( tc, "title with \"trouble\"", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "simple \"analyzer\"", lcn_field_value( field ) );

    field = lcn_list_get( lcn_document_get_fields( doc ), 1 );

    CuAssertStrEquals( tc, "text", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "open", lcn_field_value( field ) );

    field = lcn_list_get( lcn_document_get_fields( doc ), 3 );
    CuAssertStrEquals( tc, "unicodefield", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "schöne grüße передача", lcn_field_value( field ) );

    LCN_TEST( lcn_document_dump_iterator_next( it, &doc, pool ) );

    field = lcn_list_get( lcn_document_get_fields(
                              doc ), 0 );
    CuAssertStrEquals( tc, "text", lcn_field_name( field ) );
    CuAssertStrEquals( tc, "open", lcn_field_value( field ) );

    field = lcn_list_get( lcn_document_get_fields( doc ), 1 );
    CuAssertStrEquals( tc, "data", lcn_field_name( field ) );



    LCN_TEST_STATUS( lcn_document_dump_iterator_next( it, &doc, pool ), LCN_ERR_ITERATOR_NO_NEXT );

    apr_pool_destroy( pool );
}

static void
test_binary_field( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_document_t* doc;
    lcn_field_type_t ft = {0};
    lcn_field_t *string_field = NULL;
    lcn_field_t *binary_field, *binary_field2;
    lcn_list_t *list;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_document_create ( &doc, pool ) );
    LCN_TEST( lcn_field_type_set_stored( &ft, LCN_TRUE ) );
    CuAssertTrue( tc, lcn_field_type_is_stored( &ft ) );

    LCN_TEST( lcn_field_create( &string_field,
                                "string",
                                bin_val,
                                &ft,
                                pool ) );

    CuAssertPtrNotNull( tc, string_field );
    CuAssertTrue( tc, lcn_field_is_stored( string_field ) );
    CuAssertStrEquals( tc, lcn_field_value( string_field ), bin_val );

    LCN_TEST( lcn_field_type_set_binary( &ft, LCN_TRUE ) );
    CuAssertTrue( tc, lcn_field_type_is_binary( &ft ) );

    LCN_TEST( lcn_field_create_binary( &binary_field,
                                       "binary",
                                       bin_val,
                                       strlen(bin_val) + 1,
                                       pool ));

    LCN_TEST( lcn_field_create_binary( &binary_field2,
                                       "binary",
                                       bin_val2,
                                       strlen(bin_val2) + 1,
                                       pool ) );

    LCN_TEST( lcn_document_add_field( doc, string_field ));
    LCN_TEST( lcn_document_add_field( doc, binary_field ));

    list = lcn_document_get_fields( doc );
    CuAssertIntEquals( tc, 2, lcn_list_size( list ) );

    {
        char *binary_field_value = NULL, *binary_test = NULL,
             *binary_test2 = NULL, *string_test = NULL;
        unsigned int len = lcn_field_size(binary_field);


        LCN_TEST( lcn_field_binary_value( binary_field,
                                          &binary_field_value,
                                          &len,
                                          pool ) );

        CuAssertTrue( tc, binary_field_value != NULL );
        CuAssertTrue( tc, lcn_field_is_stored( binary_field ) );
        CuAssertTrue( tc, !lcn_field_is_indexed( binary_field ) );
        CuAssertTrue( tc, !lcn_field_is_tokenized( binary_field ) );

        LCN_TEST( lcn_document_get_binary_field_value( doc,
                                                       lcn_field_name( binary_field ),
                                                       &binary_test,
                                                       &len,
                                                       pool ) );

        CuAssertStrEquals( tc, binary_test, binary_field_value );

        LCN_TEST( lcn_document_get( doc,
                                    &string_test,
                                    lcn_field_name( string_field ),
                                    pool ) );

        CuAssertStrEquals( tc, string_test, binary_field_value );

        LCN_TEST( lcn_document_add_field( doc, binary_field2 ));

        list = lcn_document_get_fields( doc );
        CuAssertIntEquals( tc, 3, lcn_list_size( list ) );

        LCN_TEST( lcn_document_get_binary_field_values( doc,
                                                        "binary",
                                                        &list,
                                                        pool ) );

        CuAssertIntEquals( tc, 2, lcn_list_size( list ) );

        binary_test = lcn_list_get( list, 0 );
        binary_test2 = lcn_list_get( list, 1 );

        CuAssertTrue( tc, 0 != strcmp( binary_test, binary_test2 ) );
        CuAssertTrue( tc, 0 == strcmp( binary_test, bin_val ) );
        CuAssertTrue( tc, 0 == strcmp( binary_test2, bin_val2 ) );
    }

    apr_pool_destroy( pool );
}


#if 0
  public void testBinaryField() throws Exception {

// -->

    Notice: RemoveField not implemented.

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
