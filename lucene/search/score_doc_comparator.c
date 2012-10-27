#include "score_doc_comparator.h"
#include "index_reader.h"

static int
lcn_score_doc_int_comparator_compare( lcn_score_doc_comparator_t *comparator,
                                      lcn_score_doc_t *score_doc_a,
                                      lcn_score_doc_t *score_doc_b )
{
    int a = comparator->field_order->arr[ score_doc_a->doc ];
    int b = comparator->field_order->arr[ score_doc_b->doc ];

    return ( a<b ? -1 : ( b<a ? 1 : 0 ));
}

apr_status_t
lcn_score_doc_int_comparator_create( lcn_score_doc_comparator_t **comparator,
                                     lcn_index_reader_t *index_reader,
                                     const char *field_name,
                                     int default_val,
                                     apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_score_doc_comparator_t *cmp;

        LCNPV( cmp = apr_pcalloc( pool, sizeof( lcn_score_doc_comparator_t) ), APR_ENOMEM );
        LCNCE( lcn_index_reader_get_int_field_values( index_reader, &(cmp->field_order), field_name, default_val ));
        cmp->compare = lcn_score_doc_int_comparator_compare;

        *comparator = cmp;
    }
    while(0);

    return s;
}

int lcn_score_doc_comparator_compare( lcn_score_doc_comparator_t *comparator,
                                      lcn_score_doc_t *score_doc_a,
                                      lcn_score_doc_t *score_doc_b )
{
    return comparator->compare( comparator, score_doc_a, score_doc_b );
}
