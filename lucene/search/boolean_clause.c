#include "boolean_clause.h"

static void
lcn_boolean_clause_set_fields( lcn_boolean_clause_t* clause,
                               lcn_boolean_clause_occur_t occur )
{
    clause->occur = occur;

    switch( clause->occur )
    {
        case LCN_BOOLEAN_CLAUSE_MUST:
            clause->required   = TRUE;
            clause->prohibited = FALSE;
            break;
        case LCN_BOOLEAN_CLAUSE_MUST_NOT:
            clause->required   = FALSE;
            clause->prohibited = TRUE;
            break;
        default:
            clause->required   = FALSE;
            clause->prohibited = FALSE;
    }

}

lcn_boolean_clause_occur_t
lcn_boolean_clause_occur( lcn_boolean_clause_t* clause )
{
    return clause->occur;
}

void
lcn_boolean_clause_set_occur( lcn_boolean_clause_t* clause,
                              lcn_boolean_clause_occur_t occur )
{
    lcn_boolean_clause_set_fields( clause, occur );
}


lcn_bool_t
lcn_boolean_clause_is_prohibited( lcn_boolean_clause_t* clause )
{
    return clause->prohibited;
}

lcn_bool_t
lcn_boolean_clause_is_required( lcn_boolean_clause_t* clause )
{
    return clause->required;
}



lcn_bool_t
lcn_boolean_clause_equal( lcn_boolean_clause_t* a,
                          lcn_boolean_clause_t* b )
{
    // TODO: Query equals

    return ((lcn_bool_t) a->occur == b->occur );
}

apr_status_t
lcn_boolean_clause_to_string( lcn_boolean_clause_t* clause,
                              char** result,
                              apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        char* str;

        switch( clause->occur )
        {
        case LCN_BOOLEAN_CLAUSE_MUST:
            str = "+";
            break;
        case LCN_BOOLEAN_CLAUSE_MUST_NOT:
            str = "-";
            break;
        default:
            str = "";
        }

        LCNPV( *result = apr_pstrdup( pool,  str ),
               APR_ENOMEM );
    }
    while( FALSE );

    return s;
}

void
lcn_boolean_clause_set_query( lcn_boolean_clause_t* clause,
                              lcn_query_t* query )
{
    clause->query = query;
}

lcn_query_t*
lcn_boolean_clause_query( lcn_boolean_clause_t* clause )
{
    return clause->query;
}


apr_status_t
lcn_boolean_clause_create( lcn_boolean_clause_t** clause,
                           lcn_boolean_clause_occur_t occur,
                           lcn_query_t* query,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( (*clause) = apr_pcalloc( pool, sizeof( lcn_boolean_clause_t ) ), APR_ENOMEM );
        lcn_boolean_clause_set_fields( *clause, occur );
        (*clause)->query = query;

    }
    while( FALSE );

    return s;
}

