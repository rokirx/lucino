#include "lucene.h"
#include "fs_field.h"
#include "lcn_store.h"
#include "index_writer.h"

#define BITS2BYTE( x )  (((x)>>3) + ((x)%8 ? 1 : 0))

static apr_status_t
lcn_multi_fs_field_find_index( lcn_multi_fs_field_t *f,
                               unsigned int *index,
                               unsigned int doc_id )
{
    unsigned int i;
    unsigned int list_size = lcn_list_size( f->sub_fields );

    if ( doc_id < f->offsets[0] || lcn_list_size( f->sub_fields) == 1 )
    {
        *index = 0;
        return APR_SUCCESS;
    }

    for( i = 1; i < list_size; i++ )
    {
        if ( doc_id >= f->offsets[i-1] &&
             ( i == list_size-1 || doc_id < f->offsets[i] ))
        {
            *index = i;
            return APR_SUCCESS;
        }
    }

    return LCN_ERR_INDEX_OUT_OF_RANGE;
}

void
lcn_multi_fs_field_update_modified_flag( lcn_multi_fs_field_t* f )
{
    unsigned int i;

    f->parent.is_modified = LCN_FALSE;

    for( i = 0; i < lcn_list_size(f->sub_fields); i++ )
    {
        lcn_fs_field_t *field = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, i );

        if ( NULL != field && lcn_fs_field_is_modified( field ))
        {
            f->parent.is_modified = LCN_TRUE;
            break;
        }
    }
}

static void
lcn_multi_fs_field_update_docs_count( lcn_multi_fs_field_t* f )
{
    unsigned int last_index = lcn_list_size( f->sub_fields ) - 1;
    lcn_fs_field_t *last_field = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, last_index );

    f->parent.docs_count = (last_field ? lcn_fs_field_docs_count( last_field ) : 0 );

    if ( last_index > 0 )
    {
        f->parent.docs_count += f->offsets[last_index-1];
    }
}


static apr_status_t
lcn_multi_fs_field_set_value( lcn_fs_field_t* base_field,
                              const char *val,
                              unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;
        unsigned int index;
        unsigned int sub_fields_size = sub_fields_size = lcn_list_size( f->sub_fields );

        LCNCE( lcn_multi_fs_field_find_index( f, &index, doc_id ));

        LCNASSERT( ( index+1 == sub_fields_size) || (doc_id < lcn_fs_field_docs_count( base_field )),
                   LCN_ERR_INDEX_OUT_OF_RANGE );

        LCNCE( lcn_fs_field_set_value((lcn_fs_field_t*) lcn_list_get( f->sub_fields, index ),
                                      val,
                                      index == 0 ? doc_id : doc_id - f->offsets[index-1] ));
        lcn_multi_fs_field_update_docs_count( f );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_fs_field_set_int_value( lcn_fs_field_t *base_field,
                                  unsigned int val,
                                  unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;
        unsigned int index;
        unsigned int sub_fields_size = sub_fields_size = lcn_list_size( f->sub_fields );
        lcn_fs_field_t *subf;

        LCNCE( lcn_multi_fs_field_find_index( f, &index, doc_id ));
        LCNASSERT( ( index+1 == sub_fields_size) || (doc_id < lcn_fs_field_docs_count( base_field )),
                   LCN_ERR_INDEX_OUT_OF_RANGE );

        /* TODO? we do not implemenent setting values on null fields */
        LCNASSERT( NULL != (subf = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, index )),
                   LCN_ERR_UNSUPPORTED_OPERATION );

        LCNCE( lcn_fs_field_set_int_value( subf,
                                           val,
                                           index == 0 ? doc_id : doc_id - f->offsets[index-1] ));
        lcn_multi_fs_field_update_docs_count( f );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_fs_field_value( lcn_fs_field_t *base_field,
                          char *val,
                          unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;
        unsigned int index;
        LCNCE( lcn_multi_fs_field_find_index( f, &index, doc_id ));
        lcn_fs_field_value( (lcn_fs_field_t*) lcn_list_get( f->sub_fields, index ),
                            val,
                            index == 0 ? doc_id : doc_id - f->offsets[index-1] );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_fs_field_int_value( lcn_fs_field_t *base_field,
                              unsigned int *val,
                              unsigned int doc_id )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;
        unsigned int index;
        lcn_fs_field_t *subf;
        LCNCE( lcn_multi_fs_field_find_index( f, &index, doc_id ));
        subf = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, index );

        if ( NULL != subf )
        {
            lcn_fs_field_int_value( subf,
                                    val,
                                    index == 0 ? doc_id : doc_id - f->offsets[index-1] );
        }
        else
        {
            *val = lcn_fs_field_default_to_int( f->parent.data_size, f->parent.default_value );
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_fs_accessor_get_int_value( lcn_field_accessor_t *accessor,
                                     unsigned int *val,
                                     unsigned int doc_id )
{
    apr_status_t s;
    lcn_fs_field_t *field = (lcn_fs_field_t*) accessor;
    LCNCR( lcn_fs_field_int_value( field, val, doc_id ));
    return APR_SUCCESS;
}

static apr_status_t
lcn_multi_fs_field_commit( lcn_fs_field_t *base_field,
                           apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i = 0;
    lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;

    for( i = 0; i < lcn_list_size(f->sub_fields); i++ )
    {
        lcn_fs_field_t *field = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, i );

        if ( NULL != field && lcn_fs_field_is_modified( field ))
        {
            LCNCE( lcn_fs_field_commit( field, pool ))
        }
    }

    return s;
}

static apr_status_t
lcn_multi_fs_field_close( lcn_fs_field_t *base_field,
                          apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i = 0;
    lcn_multi_fs_field_t *f = (lcn_multi_fs_field_t*) base_field;

    for( i = 0; i < lcn_list_size(f->sub_fields); i++ )
    {
        lcn_fs_field_t *field = (lcn_fs_field_t*) lcn_list_get( f->sub_fields, i );

        if ( NULL != field )
        {
            LCNCE( lcn_fs_field_close( field, pool ))
        }
    }

    return s;
}


apr_status_t
lcn_multi_fs_field_create_by_subfields( lcn_fs_field_t **field,
                                        lcn_list_t *sub_fields,
                                        unsigned int *offsets,
                                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_multi_fs_field_t *new_field = lcn_object_create( lcn_multi_fs_field_t, pool );
    lcn_bool_t is_init = LCN_FALSE;
    lcn_fs_field_t *first_field = NULL;

    do
    {
        unsigned int i;
        unsigned int last_docs_count = 0;

        LCNCE( lcn_list_create( &(new_field->sub_fields), lcn_list_size( sub_fields ), pool ));
        new_field->offsets = apr_palloc( pool, lcn_list_size( sub_fields ) * sizeof(unsigned int));
        LCNASSERT( 0 == offsets[0], LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET );
        new_field->offsets[0] = 0;
        new_field->parent.pool = pool;

        for( i = 0; i < lcn_list_size( sub_fields ); i++ )
        {
            lcn_fs_field_t *field = (lcn_fs_field_t*) lcn_list_get( sub_fields, i );

            LCNCE( lcn_list_add( new_field->sub_fields, field ));

            if ( i > 0 )
            {
                new_field->offsets[i-1] = offsets[i];
            }

            if( is_init )
            {
                if ( NULL == field )
                {
                    last_docs_count = 0;
                    continue;
                }

                LCNASSERT( lcn_fs_field_info_is_equal( first_field, field ),
                           LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );

                LCNASSERT( last_docs_count <= offsets[i] - offsets[i-1],
                           LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET );
            }
            else if ( NULL != field )
            {
                new_field->parent.data_size     = lcn_fs_field_data_size( field );
                LCNPV( new_field->parent.tmp_value = apr_pcalloc( pool, 1 + BITS2BYTE(new_field->parent.data_size)), APR_ENOMEM );
                new_field->parent.default_value = (char*) lcn_fs_field_default_val( field );
                new_field->parent.name = apr_pstrdup( pool, lcn_fs_field_name( field ));
                is_init = LCN_TRUE;
                first_field = field;
            }

            if ( NULL != field )
            {
                last_docs_count = lcn_fs_field_docs_count( field );
            }
        }

        LCNCE(s);
        LCNASSERT( is_init, LCN_ERR_INVALID_ARGUMENT );

        new_field->parent.field_value   = lcn_multi_fs_field_value;
        new_field->parent.int_value     = lcn_multi_fs_field_int_value;
        new_field->parent.set_int_value = lcn_multi_fs_field_set_int_value;
        new_field->parent.set_value     = lcn_multi_fs_field_set_value;
        new_field->parent.type          = LCN_MULTI_FS_FIELD;
        new_field->parent.commit        = lcn_multi_fs_field_commit;
        new_field->parent.close         = lcn_multi_fs_field_close;


        /* new_field->parent.accessor.get_binary_value = lcn_fs_accessor_get_binary_value; */
        new_field->parent.accessor.get_int_value    = lcn_multi_fs_accessor_get_int_value;

        lcn_multi_fs_field_update_docs_count( new_field );
        lcn_multi_fs_field_update_modified_flag( new_field );

        *field = (lcn_fs_field_t*) new_field;
    }
    while(0);

    return s;

    return APR_SUCCESS;
}
