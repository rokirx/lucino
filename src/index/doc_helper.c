#include "lucene.h"
#include "lcn_index.h"


apr_status_t
lcn_doc_helper_setup_doc( lcn_document_t *doc,
                          apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_field_t *text_field_1;
        lcn_field_type_t custom_type_1 = {0};

        //LCNCE( lcn_field_create_ft( &text_field_1, "textField1", "field one text", lcn_field_type_text_stored( &custom_type_1 ), pool ));
    }
    while(0);

    return s;
}
