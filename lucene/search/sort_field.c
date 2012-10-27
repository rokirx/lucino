#include "sort_field.h"

apr_status_t
lcn_sort_field_set_default_int_value( lcn_sort_field_t *sort_field, int val )
{
    apr_status_t s;

    if ( sort_field->type == LCN_SORT_FIELD_INT )
    {
        sort_field->default_int = val;
        return APR_SUCCESS;
    }

    LCNRM( LCN_ERR_INVALID_ARGUMENT, "Trying to set default int value on non int field" );
}

apr_status_t
lcn_sort_field_create( lcn_sort_field_t** sort_field,
                       const char* field,
                       unsigned int type,
                       lcn_bool_t reverse,
                       apr_pool_t* pool )
{
    apr_status_t s;

    if (  LCN_SORT_FIELD_INT != type )
    {
        LCNRM( LCN_ERR_UNSUPPORTED_OPERATION, "This type of sort_field not supported yet" );
    }

    do
    {
        LCNPV( *sort_field = apr_pcalloc( pool, sizeof( lcn_sort_field_t ) ), APR_ENOMEM );

        (*sort_field)->name    = lcn_atom_get_str( field );
        (*sort_field)->type    = type;
        (*sort_field)->reverse = reverse;
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_sort_field_to_string( lcn_sort_field_t *sort_field,
                          char **str,
                          apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        const char *field_type;

        switch( sort_field->type )
        {
        case LCN_SORT_FIELD_INT   : field_type = "<int>"; break;
        default: LCNRM( LCN_ERR_UNSUPPORTED_OPERATION,
                        "This type of sort_field not supported yet" );
#if 0
            /* int still is the only supported type  */
        case LCN_SORT_FIELD_SCORE : field_type = "<score>";
        case LCN_SORT_FIELD_DOC   : field_type = "<doc>";
        case LCN_SORT_FIELD_AUTO  : field_type = "<auto>";
        case LCN_SORT_FIELD_STRING: field_type = "<string>";
        case LCN_SORT_FIELD_FLOAT : field_type = "<float>";
        case LCN_SORT_FIELD_CUSTOM: field_type = "<custom>";
#endif
        }

        *str = apr_pstrcat( pool, field_type, "\"", sort_field->name, "\"",
                            sort_field->reverse ? "!":"", NULL );
    }
    while(0);

    return s;
}
