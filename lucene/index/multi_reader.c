#include "index_reader.h"
#include "index_writer.h"
#include "term_enum.h"
#include "document.h"


static apr_status_t
lcn_multi_reader_fs_field_bitvector( lcn_index_reader_t *index_reader,
                                     lcn_bitvector_t **bitvector,
                                     const char *fname,
                                     lcn_bool_t (*filter_function)( lcn_fs_field_t *, unsigned int ),
                                     apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        unsigned int bv_size;

        lcn_fs_field_t *field = (lcn_fs_field_t*) apr_hash_get( index_reader->fs_fields,
                                                                fname,
                                                                APR_HASH_KEY_STRING );

        LCNCE( lcn_index_reader_null_bitvector( index_reader, bitvector, pool ) );

        if ( NULL == field )
        {
            return s;
        }

        /* assert that the field is loaded to RAM */

        bv_size = lcn_bitvector_size( *bitvector );

        for( i = 0; i < bv_size; i++ )
        {
            unsigned int val;
            LCNCE( lcn_fs_field_int_value( (lcn_fs_field_t*) field, &val, i ));

            if ( filter_function( (lcn_fs_field_t*) field, i ))
            {
                LCNCE( lcn_bitvector_set_bit( *bitvector, i ));
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_reader_fs_field_int_bitvector( lcn_index_reader_t *reader,
                                         lcn_bitvector_t **bitvector,
                                         const char *fname,
                                         lcn_bool_t (*filter_function) (void* data, unsigned int val, unsigned int doc_order ),
                                         void *filter_data,
                                         apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        unsigned int bv_size;

        lcn_fs_field_t *field = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                                fname,
                                                                APR_HASH_KEY_STRING );

        LCNCE( lcn_index_reader_null_bitvector( reader, bitvector, pool ) );

        if ( NULL == field )
        {
            return s;
        }

        bv_size = lcn_bitvector_size( *bitvector );

        for( i = 0; i < bv_size; i++ )
        {
            unsigned int val;

            LCNCE( lcn_fs_field_int_value( (lcn_fs_field_t*) field, &val, i ));

            if ( filter_function( filter_data, val, i ))
            {
                LCNCE( lcn_bitvector_set_bit( *bitvector, i ));
            }
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_multi_reader_term_positions( lcn_index_reader_t *index_reader,
                                 lcn_term_docs_t **term_positions,
                                 apr_pool_t *pool )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    return lcn_multi_term_positions_create( term_positions,
                                            multi_reader->sub_readers,
                                            multi_reader->starts,
                                            multi_reader->sub_readers_count,
                                            pool );
}

apr_status_t
lcn_multi_reader_terms_from( lcn_index_reader_t *index_reader,
                             lcn_term_enum_t **term_enum,
                             lcn_term_t *term,
                             apr_pool_t *pool )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    return lcn_multi_term_enum_create( term_enum,
                                       multi_reader->sub_readers,
                                       multi_reader->starts,
                                       multi_reader->sub_readers_count,
                                       term,
                                       pool );
}


static apr_status_t
lcn_multi_reader_get_field_infos( lcn_index_reader_t *index_reader,
                                  lcn_list_t *field_infos,
                                  unsigned int field_option,
                                  unsigned int options_mask)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i;
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

        for( i = 0; i < multi_reader->sub_readers_count; i++ )
        {
            lcn_index_reader_t *sub_reader = multi_reader->sub_readers[ i ];
            LCNCE( lcn_index_reader_get_field_infos( sub_reader, field_infos, field_option, options_mask ) );
        }
    }
    while(0);

    return s;
}

static unsigned int
lcn_multi_reader_num_docs( lcn_index_reader_t *index_reader )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    if ( multi_reader->num_docs_count == -1 ) /* check cache */
    {
        unsigned int n = 0;              /* cache miss -- recompute */
        int i;

        for( i = 0; i < multi_reader->sub_readers_count; i++ )
        {
            n+= lcn_index_reader_num_docs( multi_reader->sub_readers[i] );
        }

        multi_reader->num_docs_count = n;
    }

    return multi_reader->num_docs_count;
}


static unsigned int
lcn_multi_reader_max_doc( lcn_index_reader_t *index_reader )
{
    return ((lcn_multi_reader_t*)index_reader)->max_doc_count;
}

static lcn_bool_t
lcn_multi_reader_has_deletions( lcn_index_reader_t *index_reader )
{
    return ((lcn_multi_reader_t*)index_reader)->has_deletions_flag;
}

/**
 * find reader for doc n
 */
static unsigned int
lcn_multi_reader_index( lcn_index_reader_t* index_reader,
                        unsigned int n )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    unsigned int lo = 0;                                      /* search starts array    */
    unsigned int hi = multi_reader->sub_readers_count - 1;    /* for first element less */

    while( hi >= lo )
    {
        unsigned int mid = (lo + hi) >> 1;
        unsigned int mid_value = multi_reader->starts[mid];

        if ( n < mid_value)
        {
            hi = mid - 1;
        }
        else if ( n > mid_value)
        {
            lo = mid + 1;
        }
        else        /* found a match */
        {
            while ( mid+1 < multi_reader->sub_readers_count &&
                    multi_reader->starts[mid+1] == mid_value )
            {
                mid++;   /* scan to last match */
            }
            return mid;
        }
    }
    return hi;
}

static lcn_bool_t
lcn_multi_reader_is_deleted( lcn_index_reader_t *index_reader,
                             unsigned int n )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    unsigned int i = lcn_multi_reader_index( index_reader, n );         /* find segment num           */
    return lcn_index_reader_is_deleted( multi_reader->sub_readers[i], /* dispatch to segment reader */
                                        n - multi_reader->starts[i] );
}

static apr_status_t
lcn_multi_reader_document( lcn_index_reader_t *index_reader,
                           lcn_document_t **document,
                           unsigned int n,
                           apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_document_t *doc;
        unsigned int i = lcn_multi_reader_index( index_reader, n );
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

        LCNCE( lcn_index_reader_document( multi_reader->sub_readers[i],
                                          &doc,
                                          n - multi_reader->starts[i],
                                          pool ) );

        if ( index_reader->fs_fields && (0 != apr_hash_count( index_reader->fs_fields )))
        {
            LCNCE( lcn_index_reader_add_fs_fields( index_reader, doc, n, pool ));
        }

        doc->index_pos = n;
        *document = doc;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_reader_set_int_value ( lcn_index_reader_t *reader,
                                 unsigned int n,
                                 const char *fname,
                                 unsigned int int_value )
{
    apr_status_t s;
    lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                        fname,
                                                        strlen(fname) );

    if ( NULL != f )
    {
        if ( reader->is_directory_reader )
        {
            unsigned int i = lcn_multi_reader_index( reader, n );
            lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) reader;

            LCNCR( lcn_fs_field_set_int_value( f, int_value, n ));
            reader->is_modified = LCN_TRUE;
            multi_reader->sub_readers[i]->is_modified = LCN_TRUE;
            f->is_modified = LCN_TRUE;

            return s;
        }
        else
        {
            unsigned int i = lcn_multi_reader_index( reader, n );
            lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) reader;

            return multi_reader->sub_readers[i]->set_int_value( multi_reader->sub_readers[i],
                                                                n - multi_reader->starts[i],
                                                                fname,
                                                                int_value );
        }
    }

    return LCN_ERR_FIELD_NOT_FOUND;
}


static apr_status_t
lcn_multi_reader_set_char_value ( lcn_index_reader_t *reader,
                                  unsigned int n,
                                  const char *fname,
                                  const char *char_value )
{
    apr_status_t s;
    lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                        fname,
                                                        strlen(fname) );

    if ( NULL != f )
    {
        if ( reader->is_directory_reader )
        {
            unsigned int i = lcn_multi_reader_index( reader, n );
            lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) reader;

            LCNCR( lcn_fs_field_set_value( f, char_value, n ));
            reader->is_modified = LCN_TRUE;
            multi_reader->sub_readers[i]->is_modified = LCN_TRUE;
            f->is_modified = LCN_TRUE;

            return s;
        }
        else
        {
            unsigned int i = lcn_multi_reader_index( reader, n );
            lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) reader;

            return multi_reader->sub_readers[i]->set_char_value( multi_reader->sub_readers[i],
                                                                 n - multi_reader->starts[i],
                                                                 fname,
                                                                 char_value );
        }
    }

    return LCN_ERR_FIELD_NOT_FOUND;
}

static apr_status_t
lcn_multi_reader_do_delete( lcn_index_reader_t *index_reader, int n )
{
    apr_status_t s;

    do
    {
        unsigned int i = lcn_multi_reader_index( index_reader, n );
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;
        multi_reader->num_docs_count = -1;
        LCNCE( lcn_index_reader_delete( multi_reader->sub_readers[i], n - multi_reader->starts[i] ) );
        multi_reader->has_deletions_flag = LCN_TRUE;
    }
    while(0);

    return s;
}


static apr_status_t
lcn_multi_reader_do_undelete_all( lcn_index_reader_t *index_reader )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    for( i = 0; i < multi_reader->sub_readers_count; i++ )
    {
        LCNCE( lcn_index_reader_undelete_all( multi_reader->sub_readers[i] ) );
    }

    if ( APR_SUCCESS == s )
    {
        multi_reader->has_deletions_flag = LCN_FALSE;
        multi_reader->num_docs_count = -1;
    }

    return s;
}

static apr_status_t
/* synchronized */
lcn_multi_reader_do_close( lcn_index_reader_t *index_reader )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    for( i = 0; i < multi_reader->sub_readers_count; i++ )
    {
        apr_status_t status = lcn_index_reader_close( multi_reader->sub_readers[i] );

        if ( APR_SUCCESS == s && status )
        {
            s = status;
        }
    }

    /* close fixed sized fields */

    LCNCR( lcn_fs_field_close_fields_in_hash( index_reader->fs_fields ));

    return s;
}

static apr_status_t
lcn_multi_reader_doc_freq( lcn_index_reader_t *index_reader,
                           const lcn_term_t *term,
                           int *freq )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

        int total = 0;
        int i;
        int f;

        for( i = 0; i < multi_reader->sub_readers_count; i++ )
        {
            LCNCE( lcn_index_reader_doc_freq( multi_reader->sub_readers[i], term, &f ) );
            total += f;
        }

        if ( APR_SUCCESS == s )
        {
            *freq = total;
        }
    }
    while(0);

    return s;
}

static lcn_bool_t
lcn_multi_reader_has_norms( lcn_index_reader_t *index_reader,
                            const char *field )
{
    unsigned int i;
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    for( i = 0; i < multi_reader->sub_readers_count; i++ )
    {
        if ( lcn_index_reader_has_norms( multi_reader->sub_readers[i], field ) )
        {
            return LCN_TRUE;
        }
    }

    return LCN_FALSE;
}

static apr_status_t
lcn_multi_reader_term_docs( lcn_index_reader_t *index_reader,
                            lcn_term_docs_t **term_docs,
                            apr_pool_t *pool )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    return lcn_multi_term_docs_create( term_docs,
                                       multi_reader->sub_readers,
                                       multi_reader->starts,
                                       multi_reader->sub_readers_count,
                                       pool );
}

static apr_status_t
lcn_multi_reader_terms( lcn_index_reader_t *index_reader,
                        lcn_term_enum_t **term_enum,
                        apr_pool_t *pool )
{
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    return lcn_multi_term_enum_create( term_enum,
                                       multi_reader->sub_readers,
                                       multi_reader->starts,
                                       multi_reader->sub_readers_count,
                                       NULL,
                                       pool );
}

static apr_status_t
lcn_multi_reader_do_commit( lcn_index_reader_t *index_reader )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

    for( i = 0; i < multi_reader->sub_readers_count; i++ )
    {
        apr_status_t status = lcn_index_reader_commit( multi_reader->sub_readers[i] );

        if ( APR_SUCCESS == s && status )
        {
            s = status;
        }
    }

    if ( LCN_TRUE == index_reader->is_directory_reader )
    {
        apr_hash_index_t *hi;

        for( hi = apr_hash_first( index_reader->pool, index_reader->fs_fields); hi; hi = apr_hash_next( hi ))
        {
            lcn_fs_field_t *fs_field;
            void *vval;

            apr_hash_this( hi, NULL, NULL, &vval );
            fs_field = (lcn_fs_field_t *) vval;

            if ( NULL != fs_field && fs_field->is_modified )
            {
                LCNCE( lcn_fs_field_commit( fs_field, index_reader->pool ));
            }
        }
    }


    return s;
}

static apr_status_t
lcn_multi_reader_read_norms_to_array( lcn_index_reader_t *index_reader,
                                      lcn_byte_array_t* result_norms,
                                      unsigned int offset,
                                      const char *field )
{
    apr_status_t s;

    do
    {
        lcn_byte_array_t *norm_bytes;
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

        LCNASSERT( lcn_index_reader_has_norms( index_reader, field ), LCN_ERR_NORMS_NOT_FOUND );

        norm_bytes = (lcn_byte_array_t*) apr_hash_get( multi_reader->parent.norms,
                                                       field,
                                                       APR_HASH_KEY_STRING );

        if ( NULL != norm_bytes ) /* cache hit */
        {
            memcpy( result_norms->arr + offset, norm_bytes->arr, norm_bytes->length );
        }
        else
        {
            unsigned int i;

            for( i = 0; i < multi_reader->sub_readers_count; i++ )
            {
                lcn_index_reader_t *sub_reader = multi_reader->sub_readers[ i ];

                LCNCE( lcn_index_reader_read_norms_to_array( sub_reader,
                                                             result_norms,
                                                             multi_reader->starts[ i ],
                                                             field ));
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_reader_norms( lcn_index_reader_t *index_reader,
                        lcn_byte_array_t** result_norms,
                        const char *field )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *norms_pool = NULL;

    do
    {
        lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) index_reader;

        lcn_byte_array_t *norm_bytes = (lcn_byte_array_t*) apr_hash_get( index_reader->norms,
                                                                         field,
                                                                         APR_HASH_KEY_STRING );
        if ( NULL == norm_bytes )
        {
            unsigned int i;
            unsigned int max_doc;

            if( !lcn_index_reader_has_norms( index_reader, field ) )
            {
                s = LCN_ERR_NORMS_NOT_FOUND;
                break;
            }

            max_doc = lcn_index_reader_max_doc( index_reader );

            LCNCE( apr_pool_create( &norms_pool, index_reader->pool ));

            LCNCE( lcn_byte_array_create( &(norm_bytes),
                                          max_doc,
                                          norms_pool ));

            for( i = 0; i < multi_reader->sub_readers_count; i++ )
            {
                lcn_index_reader_t *sub_reader = multi_reader->sub_readers[ i ];
                lcn_byte_array_t *sub_norms;

                s = lcn_index_reader_norms( sub_reader, &sub_norms, field );

                if ( LCN_ERR_NORMS_NOT_FOUND == s )
                {
                    s = APR_SUCCESS;
                    continue;
                }

                LCNCE( s );

                memcpy( norm_bytes->arr + multi_reader->starts[ i ],
                        sub_norms->arr,
                        sub_norms->length );
            }

            if ( s )
            {
                break;
            }

            apr_hash_set( index_reader->norms, apr_pstrdup( norms_pool, field ), strlen(field), norm_bytes );
        }

        *result_norms = norm_bytes;
        index_reader->norms_pool = norms_pool;
    }
    while(0);

    if ( s )
    {
        if ( NULL != norms_pool )
        {
            apr_pool_destroy( norms_pool );
        }
    }

    return s;
}

static apr_status_t
lcn_multi_reader_add_fs_field_def( lcn_index_reader_t *reader,
                                   lcn_field_t *field )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    lcn_multi_reader_t *multi_reader = (lcn_multi_reader_t*) reader;

    do
    {
        lcn_fs_field_t *m_field = apr_hash_get( multi_reader->parent.fs_fields,
                                                lcn_field_name( field ),
                                                strlen(lcn_field_name( field )));

        if ( NULL == m_field )
        {
            if ( reader->is_directory_reader )
            {
                LCNCE( lcn_directory_fs_field_create( &m_field,
                                                      lcn_field_name( field ),
                                                      0, /* assume docs count 0 */
                                                      lcn_field_size( field ),
                                                      reader->directory,
                                                      reader->pool ));
                apr_hash_set( multi_reader->parent.fs_fields,
                              lcn_field_name( field ),
                              strlen( lcn_field_name( field )), m_field );
            }
            else
            {
                lcn_list_t *sub_fields_list;

                LCNCE( lcn_list_create( &sub_fields_list, 10, reader->pool ));

                for( i = 0; i < multi_reader->sub_readers_count; i++ )
                {
                    lcn_fs_field_t *f;
                    lcn_index_reader_t *sub_reader = multi_reader->sub_readers[ i ];

                    LCNCE( lcn_index_reader_add_fs_field_def( sub_reader, field ));
                    f = apr_hash_get( sub_reader->fs_fields, lcn_field_name( field ), strlen( lcn_field_name( field )));
                    LCNCE( lcn_list_add( sub_fields_list, f ));
                    LCNCE( lcn_multi_fs_field_create_by_subfields( &m_field,
                                                                   sub_fields_list,
                                                                   multi_reader->starts,
                                                                   multi_reader->parent.pool ));
                    apr_hash_set( multi_reader->parent.fs_fields,
                                  lcn_field_name( field ),
                                  strlen( lcn_field_name( field )), m_field );
                }
            }
        }
        else
        {
            lcn_fs_field_t *fs_field;

            LCNCE( lcn_directory_fs_field_create( &fs_field,
                                                  lcn_field_name( field ),
                                                  0, /* assume docs count 0 */
                                                  lcn_field_size( field ),
                                                  NULL,
                                                  reader->pool ));

            LCNASSERT( lcn_fs_field_info_is_equal( fs_field, m_field ),
                       LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );
        }
    }
    while(0);

    return s;
}

static lcn_bool_t
lcn_multi_reader_has_changes( lcn_index_reader_t *index_reader )
{

    if ( index_reader->is_modified )
    {
        return LCN_TRUE;
    }

    {
        apr_hash_index_t *hi;

        for( hi = apr_hash_first( index_reader->pool, index_reader->fs_fields); hi; hi = apr_hash_next( hi ))
        {
            lcn_fs_field_t *fs_field;
            void *vval;

            apr_hash_this( hi, NULL, NULL, &vval );
            fs_field = (lcn_fs_field_t *) vval;

            if ( fs_field->is_modified )
            {
                index_reader->is_modified = LCN_TRUE;
                break;
            }
        }
    }

    return index_reader->is_modified;
}

static apr_status_t
lcn_multi_reader_initialize( lcn_multi_reader_t *multi_reader,
                             lcn_index_reader_t **sub_readers,
                             unsigned int sub_readers_count )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    do
    {
        unsigned int i;
        apr_hash_t *fields_hash;
        apr_hash_index_t *hi;

        multi_reader->sub_readers = sub_readers;
        multi_reader->sub_readers_count = sub_readers_count;

        LCNPV( multi_reader->starts = (unsigned int*) apr_palloc(
                   multi_reader->parent.pool, sizeof(unsigned int)  * (sub_readers_count + 1) ), APR_ENOMEM );

        LCNCE( apr_pool_create( &cp, multi_reader->parent.pool ));

        LCNPV( fields_hash = apr_hash_make( cp ), APR_ENOMEM );

        for( i = 0; i < sub_readers_count; i++ )
        {
            multi_reader->starts[i] = multi_reader->max_doc_count;
            multi_reader->max_doc_count += lcn_index_reader_max_doc( sub_readers[i] );

            if ( lcn_index_reader_has_deletions( sub_readers[i] ) )
            {
                multi_reader->has_deletions_flag = LCN_TRUE;
            }

            /* iterate through the fs_fields of the i-th subreader to complete */
            /* the combined fs_fields of the multi_reader                      */

            for( hi = apr_hash_first( cp, sub_readers[i]->fs_fields); hi; hi = apr_hash_next( hi ))
            {
                lcn_fs_field_t *sub_field;
                void *vval;
                const void *vkey;
                char *sub_field_name;
                lcn_list_t *sub_fields_list;

                apr_hash_this( hi, &vkey, NULL, &vval );
                sub_field = (lcn_fs_field_t *) vval;
                sub_field_name = (char*) vkey;

                /* check if there is a list initialized for sub_field_name */

                if ( NULL != ( sub_fields_list = apr_hash_get( fields_hash, sub_field_name, strlen(sub_field_name))))
                {
                    unsigned int sub_fields_list_size;

                    /* if there are no missing field definitions, then the length of */
                    /* the list should be i                                          */

                    if (( sub_fields_list_size = lcn_list_size( sub_fields_list )) < i )
                    {
                        unsigned int j;

                        for( j = 0; j < i - sub_fields_list_size; j++ )
                        {
                            LCNCE( lcn_list_add( sub_fields_list, NULL ));
                        }
                    }

                    /* add the field definition */

                    LCNCE( lcn_list_add( sub_fields_list, sub_field ));
                }
                else
                {
                    /* initialize new list properly and store it in the hash */

                    unsigned int j = 0;

                    LCNCE( lcn_list_create( &sub_fields_list, 10, cp ));
                    apr_hash_set( fields_hash, sub_field_name, strlen(sub_field_name), sub_fields_list );

                    /* in case i > 0 we shuld fill the list with placeholder fields */

                    for( j = 0; j < i; j++ )
                    {
                        LCNCE( lcn_list_add( sub_fields_list, NULL ));
                    }

                    LCNCE(s);

                    LCNCE( lcn_list_add( sub_fields_list, sub_field ));
                }

                LCNCE(s);
            }

            LCNCE( s );
        }

        /* Some lists may be incomplete still     */
        /* iterate through hash and define fields */

        for( hi = apr_hash_first( cp, fields_hash); hi; hi = apr_hash_next( hi ))
        {
            const void *vkey;
            void *vval;
            lcn_list_t *sub_field_list;
            const char* sub_field_name;
            unsigned int listlen;
            lcn_fs_field_t *m_field;

            apr_hash_this( hi, &vkey, NULL, &vval );

            sub_field_name = (char*) vkey;
            sub_field_list = (lcn_list_t *) vval;

            if ( (listlen = lcn_list_size( sub_field_list )) < sub_readers_count )
            {
                int j;

                for( j = 0; j < sub_readers_count - listlen; j++ )
                {
                    LCNCE( lcn_list_add( sub_field_list, NULL ));
                }
            }

            LCNASSERT( sub_readers_count == lcn_list_size( sub_field_list ), LCN_ERR_INVALID_ARGUMENT );

            /* now the field list has correct size. Initialize the composed field */
            /* and store it in the multi_reader hash                              */

            LCNCE( lcn_multi_fs_field_create_by_subfields( &m_field,
                                                           sub_field_list,
                                                           multi_reader->starts,
                                                           multi_reader->parent.pool ));
            apr_hash_set( multi_reader->parent.fs_fields, sub_field_name, strlen(sub_field_name), m_field );
        }



        LCNCE( s );

        multi_reader->starts[i] = multi_reader->max_doc_count;
        multi_reader->num_docs_count = -1;

        multi_reader->parent.doc_freq        = lcn_multi_reader_doc_freq;
        multi_reader->parent.num_docs        = lcn_multi_reader_num_docs;
        multi_reader->parent.max_doc         = lcn_multi_reader_max_doc;
        multi_reader->parent.has_deletions   = lcn_multi_reader_has_deletions;
        multi_reader->parent.is_deleted      = lcn_multi_reader_is_deleted;
        multi_reader->parent.document        = lcn_multi_reader_document;
        multi_reader->parent.do_delete       = lcn_multi_reader_do_delete;
        multi_reader->parent.do_undelete_all = lcn_multi_reader_do_undelete_all;
        multi_reader->parent.do_close        = lcn_multi_reader_do_close;
        multi_reader->parent.do_commit       = lcn_multi_reader_do_commit;
        multi_reader->parent.has_norms       = lcn_multi_reader_has_norms;
        multi_reader->parent.terms           = lcn_multi_reader_terms;
        multi_reader->parent.terms_from      = lcn_multi_reader_terms_from;
        multi_reader->parent.term_docs       = lcn_multi_reader_term_docs;
        multi_reader->parent.term_positions  = lcn_multi_reader_term_positions;
        multi_reader->parent.get_field_infos = lcn_multi_reader_get_field_infos;
        multi_reader->parent.get_norms       = lcn_multi_reader_norms;

        multi_reader->parent.read_norms_to_array    = lcn_multi_reader_read_norms_to_array;
        multi_reader->parent.fs_field_bitvector     = lcn_multi_reader_fs_field_bitvector;
        multi_reader->parent.fs_field_int_bitvector = lcn_multi_reader_fs_field_int_bitvector;
        multi_reader->parent.set_int_value          = lcn_multi_reader_set_int_value;
        multi_reader->parent.set_char_value         = lcn_multi_reader_set_char_value;
        multi_reader->parent.add_fs_field_def       = lcn_multi_reader_add_fs_field_def;
        multi_reader->parent.has_changes            = lcn_multi_reader_has_changes;

    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_multi_reader_create( lcn_index_reader_t **index_reader,
                         lcn_directory_t *directory,
                         lcn_segment_infos_t *segment_infos,
                         lcn_bool_t close_directory,
                         lcn_index_reader_t **readers,
                         apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_multi_reader_t *multi_reader = lcn_object_create( lcn_multi_reader_t, pool );

        LCNCE( lcn_index_reader_create_as_owner( lcn_cast_index_reader(multi_reader),
                                                 directory,
                                                 segment_infos,
                                                 close_directory,
                                                 pool ) );

        LCNCE( lcn_multi_reader_initialize( multi_reader,
                                            readers,
                                            lcn_segment_infos_size( segment_infos ) ));
        *index_reader = lcn_cast_index_reader(multi_reader);
    }
    while(0);

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_reader_create_by_sub_readers( lcn_index_reader_t **reader,
                                        lcn_list_t *sub_readers,
                                        apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_multi_reader_t *multi_reader;
        lcn_index_reader_t **readers;
        unsigned int i;
        lcn_index_reader_t *r = (lcn_index_reader_t*) lcn_list_get( sub_readers, 0 );

        if ( 1 == lcn_list_size( sub_readers ) )
        {
            *reader = r;
            break;
        }

        LCNPV( readers = apr_pcalloc( pool, lcn_list_size( sub_readers ) * sizeof(lcn_index_reader_t*) ), APR_ENOMEM );

        for( i = 0; i < lcn_list_size( sub_readers ); i++ )
        {
            readers[ i ] =(lcn_index_reader_t*) lcn_list_get( sub_readers, i );
        }

        multi_reader = lcn_object_create( lcn_multi_reader_t, pool );

        LCNCE( lcn_index_reader_init( lcn_cast_index_reader(multi_reader),
                                      lcn_index_reader_directory( r ),
                                      NULL,      /* segment_infos   */
                                      LCN_TRUE,  /* close directory */
                                      LCN_FALSE, /* directory_owner */
                                      pool ));

        /* this also collects info about existing fs_fields */

        LCNCE( lcn_multi_reader_initialize( multi_reader, readers, lcn_list_size( sub_readers ) ));

        *reader = lcn_cast_index_reader(multi_reader);

#if 0
        {
            apr_hash_index_t *hi;

            for( hi = apr_hash_first( pool, (*reader)->fs_fields); hi; hi = apr_hash_next( hi ))
            {
                lcn_fs_field_t *fs_field;
                //lcn_field_t *field;
                void *vval;

                apr_hash_this( hi, NULL, NULL, &vval );
                fs_field = (lcn_fs_field_t *) vval;
            }
        }
#endif

    }
    while(0);

    return s;
}
