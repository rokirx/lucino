#include "lucene.h"
#include "lcn_index.h"
#include "field.h"
#include "fs_field.h"

const char *
lcn_field_name( lcn_field_t *field )
{
    return field->name;
}

apr_status_t
lcn_field_int_value( lcn_field_t *field,
                     unsigned int *val )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        LCNASSERTM( lcn_field_is_fixed_size( field ), LCN_ERR_INVALID_ARGUMENT, field->name );
        LCNCE( field->accessor->get_int_value( field->accessor, val, field->doc_id ));
    }
    while(0);

    return s;
}


apr_status_t
lcn_field_binary_value( lcn_field_t *field,
                        char **value,
                        unsigned int *len,
                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( field->is_lazy )
        {
            field->accessor->get_binary_value( field->accessor,
                                               value,
                                               len,
                                               field->doc_id,
                                               pool );
        }
        else
        {
            LCNPV( *value = (char*) apr_palloc( pool, field->size * sizeof(char)), APR_ENOMEM );
            memcpy( *value, field->value, field->size );
            *len = field->size;
        }
    }
    while(0);

    return s;
}

const char *
lcn_field_value( lcn_field_t *field )
{
    return field->value;
}

lcn_bool_t
lcn_field_is_indexed( lcn_field_t *field )
{
    return lcn_field_type_is_indexed( &(field->field_type) );
}

lcn_bool_t
lcn_field_is_tokenized( lcn_field_t *field )
{
    return lcn_field_type_is_tokenized( &(field->field_type) );
}

lcn_bool_t
lcn_field_is_stored( lcn_field_t *field )
{
    return lcn_field_type_is_stored( &(field->field_type) );
}

lcn_bool_t
lcn_field_is_fixed_size( lcn_field_t *field )
{
    return lcn_field_type_is_fixed_size( &(field->field_type) );
}

const char *
lcn_field_default_value( lcn_field_t *field )
{
    return  field->default_value;
}

lcn_bool_t
lcn_field_is_binary( lcn_field_t *field )
{
    return lcn_field_type_is_binary( &(field->field_type) );
}

lcn_bool_t
lcn_field_is_term_vector_stored( lcn_field_t *field )
{
    return lcn_field_type_is_store_term_vectors( &(field->field_type) );
}

lcn_bool_t
lcn_field_store_position_with_term_vector( lcn_field_t *field )
{
    return lcn_field_type_is_store_term_vector_positions( &(field->field_type) );
}

lcn_bool_t
lcn_field_store_offset_with_term_vector( lcn_field_t *field )
{
    return lcn_field_type_is_store_term_vector_offsets( &(field->field_type) );
}

lcn_bool_t
lcn_field_omit_norms( lcn_field_t *field )
{
    return lcn_field_type_is_omit_norms( &(field->field_type) );
}

unsigned int
lcn_field_size( lcn_field_t *field )
{
    if ( lcn_field_is_binary( field ) || lcn_field_is_fixed_size( field ))
    {
        return field->size;
    }

    return strlen( field->value );
}

float
lcn_field_get_boost( lcn_field_t *field )
{
    return field->boost;
}

void
lcn_field_set_analyzer( lcn_field_t *field, lcn_analyzer_t *analyzer )
{
    field->analyzer = analyzer;
}

apr_status_t
lcn_field_get_analyzer( lcn_field_t *field,
                        lcn_analyzer_t **analyzer )
{
    if ( NULL != field->analyzer )
    {
        *analyzer = field->analyzer;
        return APR_SUCCESS;
    }

    return LCN_ERR_FIELD_ANALYZER_NOT_INITIALIZED;
}

static apr_status_t
lcn_field_create_impl( lcn_field_t **field,
                       const char *name,
                       const char *value,
                       unsigned int value_size,
                       lcn_field_type_t *ft,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *field = (lcn_field_t*) apr_pcalloc( pool, sizeof(lcn_field_t) ), APR_ENOMEM );

        (*field)->boost      = (float) 1.0;
        (*field)->name       = lcn_atom_get_str ( name );
        (*field)->value      = (char*)value;
        (*field)->field_type = *ft;
        (*field)->analyzer   = NULL;

        if ( lcn_field_type_is_binary( ft ) )
        {
            (*field)->size = value_size;
        }

        if ( !lcn_field_type_is_indexed( ft ) && lcn_field_type_is_omit_norms( ft ))
        {
            s = LCN_ERR_FIELD_INFO_OMIT_NORMS_ON_UNINDEXED;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_field_create_fixed_size( lcn_field_t **field,
                             const char *name,
                             const char *value,
                             const char *default_value,
                             unsigned int size,
                             apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        char *val;
        unsigned int bytes = (size>>3) + ((size&7) ? 1 : 0 );
        lcn_field_type_t field_type = {0};

        lcn_field_type_set_stored( &field_type, LCN_TRUE );
        lcn_field_type_set_binary( &field_type, LCN_TRUE );
        lcn_field_type_set_fixed_size( &field_type, LCN_TRUE );

        LCNPV( val = (char*) apr_palloc( pool, bytes ), APR_ENOMEM );

        if ( NULL != value )
        {
            memcpy( val, value, bytes );
        }

        LCNCR( lcn_field_create_impl( field, name, val, bytes, &field_type, pool ));

        if ( NULL != default_value )
        {
            LCNPV( (*field)->default_value = (char*) apr_palloc( pool, bytes ),APR_ENOMEM );
            memcpy( (*field)->default_value, default_value, bytes );
        }

        (*field)->size = size;
    }
    while(0);

    return s;
}


apr_status_t
lcn_field_create_fixed_size_by_ints( lcn_field_t **field,
                                     const char *name,
                                     unsigned int val,
                                     unsigned int default_value,
                                     unsigned int size,
                                     apr_pool_t *pool )
{
    apr_status_t s;

    char value[ sizeof( unsigned int )];
    char defval[ sizeof( unsigned int )];

    LCNCR( lcn_fs_field_int_to_char( val, value, size ) );
    LCNCR( lcn_fs_field_int_to_char( default_value, defval, size ) );

    return lcn_field_create_fixed_size( field, name, value, defval, size, pool );
}

apr_status_t
lcn_field_create_binary( lcn_field_t **field,
                         const char *name,
                         const char *value,
                         unsigned int value_size,
                         apr_pool_t *pool )
{
    apr_status_t s;
    lcn_field_type_t ft = {0};

    (void) lcn_field_type_binary( &ft );

    LCNCR( lcn_field_create_impl( field, name, value, value_size, &ft, pool ));

    return s;
}

apr_status_t
lcn_field_create( lcn_field_t **field,
                  const char *name,
                  const char *value,
                  lcn_field_type_t *ft,
                  apr_pool_t *pool )
{
    return lcn_field_create_impl( field, name, value, strlen( value ), ft, pool );
}
