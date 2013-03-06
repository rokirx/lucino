#include "index_reader.h"
#include "fs_field.h"

#if 0

/**
 * This functions are for debugging only
 */
apr_hash_t *
lcn_index_reader_norms_hash( lcn_index_reader_t *reader )
{
    return reader->norms;
}

apr_pool_t *
lcn_index_reader_pool( lcn_index_reader_t *reader )
{
    return reader->pool;
}

#endif

apr_status_t
lcn_index_reader_add_fs_field_def( lcn_index_reader_t *index_reader,
                                   lcn_field_t * field )
{
    return index_reader->add_fs_field_def( index_reader, field );
}

apr_status_t
lcn_index_reader_add_fs_fields( lcn_index_reader_t *index_reader,
                                lcn_document_t *document,
                                unsigned int n,
                                apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        apr_hash_index_t *hi;

        for( hi = apr_hash_first( pool, index_reader->fs_fields); hi; hi = apr_hash_next( hi ))
        {
            lcn_fs_field_t *fs_field;
            lcn_field_t *field;
            void *vval;

            apr_hash_this( hi, NULL, NULL, &vval );
            fs_field = (lcn_fs_field_t *) vval;
            LCNCE( lcn_fs_field_to_field( fs_field, &field, n, pool ));
            LCNCE( lcn_document_add_field( document, field ));
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_reader_term_positions( lcn_index_reader_t *index_reader,
                                 lcn_term_docs_t **term_positions,
                                 apr_pool_t *pool )
{
    return index_reader->term_positions( index_reader, term_positions, pool );
}

apr_status_t
lcn_index_reader_term_positions_by_term( lcn_index_reader_t *index_reader,
                                         lcn_term_docs_t **term_positions,
                                         lcn_term_t *term,
                                         apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( index_reader->term_positions( index_reader, term_positions, pool ) );

    s = lcn_term_docs_seek_term( *term_positions, term );

    if( s )
    {
        if( ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ) ||
            ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) )
        {
            return s;
        }
        LCNCR( s );
    }
    return s;
}

apr_status_t
lcn_index_reader_init( lcn_index_reader_t *index_reader,
                       lcn_directory_t *directory,
                       lcn_segment_infos_t *segment_infos,
                       lcn_bool_t close_directory,
                       lcn_bool_t directory_owner,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        index_reader->pool = pool;
        index_reader->directory = directory;
        index_reader->segment_infos = segment_infos;
        index_reader->directory_owner = directory_owner;
        index_reader->close_directory = close_directory;

        LCNPV( index_reader->norms = apr_hash_make( index_reader->pool ), APR_ENOMEM );
        LCNPV( index_reader->fs_fields = apr_hash_make( index_reader->pool ), APR_ENOMEM );
    }
    while(0);

    return s;
}

apr_status_t
lcn_segment_reader_create_by_info( lcn_index_reader_t **index_reader,
                                   lcn_segment_info_t *segment_info,
                                   apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_segment_reader_create( index_reader,
                                      lcn_segment_info_directory( segment_info ),
                                      NULL,
                                      segment_info,
                                      LCN_FALSE,  /* close_directory */
                                      LCN_FALSE,  /* own_dir         */
                                      pool ) );
    return s;
}

apr_status_t
lcn_index_reader_read_norms_to_array( lcn_index_reader_t *index_reader,
                                      lcn_byte_array_t* result_norms,
                                      unsigned int offset,
                                      const char *field )
{
    return index_reader->read_norms_to_array( index_reader,
                                              result_norms,
                                              offset,
                                              field );
}

apr_status_t
lcn_index_reader_norms( lcn_index_reader_t *index_reader,
                        lcn_byte_array_t** result_norms,
                        const char *field )
{
    return index_reader->get_norms( index_reader, result_norms, field );
}

static apr_status_t
lcn_index_reader_create_by_directory_impl( lcn_index_reader_t **index_reader,
                                           lcn_directory_t *directory,
                                           lcn_bool_t close_directory,
                                           apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_segment_infos_t *infos;
        unsigned int segments_count;

        LCNCE( lcn_segment_infos_create( &infos, pool ) );
        LCNCE( lcn_segment_infos_read_directory( infos, directory ) );

        if ( 1 == (segments_count = lcn_segment_infos_size( infos ) ) )
        {
            lcn_segment_info_per_commit_t *info_pc;
            lcn_segment_info_t *info;

            LCNCE( lcn_segment_infos_get( infos, &info_pc, 0 ));
            info = lcn_segment_info_per_commit_info( info_pc );

            LCNCE( lcn_segment_reader_create( index_reader,
                                              lcn_segment_info_directory( info ),
                                              infos,
                                              info,
                                              close_directory,
                                              LCN_TRUE,  /* directory owner */
                                              pool ) );
        }
        else
        {
            lcn_index_reader_t **readers;
            unsigned int i;

            LCNPV( readers = (lcn_index_reader_t**) apr_palloc( pool, sizeof(lcn_index_reader_t*) * segments_count ),
                   APR_ENOMEM );

            for( i = 0; i < segments_count; i++ )
            {
                lcn_index_reader_t *segment_reader;
                lcn_segment_info_per_commit_t *info_pc;
                lcn_segment_info_t *info;

                LCNCE( lcn_segment_infos_get( infos, &info_pc, i ));
                info = lcn_segment_info_per_commit_info( info_pc );

                LCNCE( lcn_segment_reader_create_by_info( &segment_reader, info, pool ) );
                readers[i] = segment_reader;
            }

            LCNCE( lcn_multi_reader_create( index_reader, directory, infos, close_directory, readers, pool ));

            (*index_reader)->is_directory_reader = LCN_TRUE;
        }

        LCNCE( lcn_directory_fs_field_read_field_infos( (*index_reader)->fs_fields, directory, pool ));
    }
    while(0);

    return s;
}

/**
 * Constructor used if lcn_index_reader_t is owner of its directory.
 * If lcn_index_reader_t is owner of its directory, it locks its
 * directory in case of write operations.
 */
apr_status_t
lcn_index_reader_create_as_owner( lcn_index_reader_t *index_reader,
                                  lcn_directory_t *directory,
                                  lcn_segment_infos_t *segment_infos,
                                  lcn_bool_t close_directory,
                                  apr_pool_t *pool )
{
    return lcn_index_reader_init( index_reader,
                                  directory,
                                  segment_infos,
                                  close_directory,
                                  LCN_TRUE,
                                  pool );
}

apr_status_t
lcn_index_reader_create_by_path( lcn_index_reader_t **index_reader,
                                 const char *path,
                                 apr_pool_t *pool )
{
    apr_status_t s;
    lcn_directory_t *dir = NULL;

    do
    {
        LCNCM( lcn_fs_directory_create( &dir, path, LCN_FALSE, pool ), path );
        LCNCE( lcn_index_reader_create_by_directory( index_reader, dir, LCN_TRUE, pool ));
    }
    while(0);

    if ( s && dir )
    {
        apr_status_t stat = lcn_directory_close( dir );

        if ( APR_SUCCESS != stat && APR_SUCCESS == s )
        {
            s = stat;
        }
    }

    return s;
}

apr_status_t
lcn_index_reader_create_by_directory( lcn_index_reader_t **index_reader,
                                      lcn_directory_t* dir,
                                      lcn_bool_t close_dir,
                                      apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *p;

    LCNCR( apr_pool_create( &p, pool ) );
    LCNCR( lcn_index_reader_create_by_directory_impl( index_reader, dir, close_dir, p ) );

    return s;
}

unsigned int
lcn_index_reader_num_docs( lcn_index_reader_t *index_reader )
{
    return index_reader->num_docs( index_reader );
}

unsigned int
lcn_index_reader_max_doc( lcn_index_reader_t *index_reader )
{
    return index_reader->max_doc( index_reader );
}

lcn_bool_t
lcn_index_reader_has_deletions( lcn_index_reader_t *index_reader )
{
    return index_reader->has_deletions( index_reader );
}

void
lcn_index_reader_set_log_stream( lcn_index_reader_t *index_reader,
                                 FILE *log_stream )
{
    index_reader->log_stream = log_stream;
}

apr_status_t
lcn_index_reader_delete( lcn_index_reader_t *index_reader, int doc_num )
{
#if 0
    if(directoryOwner)
      aquireWriteLock();
#endif
    apr_status_t s = index_reader->do_delete( index_reader, doc_num );

    if ( NULL != index_reader->log_stream )
    {
        fprintf( index_reader->log_stream, "Index Reader delete document %d\n", doc_num );
    }

    if ( APR_SUCCESS == s )
    {
        index_reader->is_modified = LCN_TRUE;
    }

    return s;
}

apr_status_t
lcn_index_reader_delete_documents( lcn_index_reader_t *index_reader,
                                   lcn_term_t *term,
                                   unsigned int *docs_deleted )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;
    lcn_term_docs_t *term_docs = NULL;

    if ( NULL != index_reader->log_stream )
    {
        fprintf( index_reader->log_stream, "Index Reader delete documents by term <%s:%s>\n",
                 lcn_term_field( term), lcn_term_text( term ));
    }

    LCNCR( apr_pool_create( &pool, index_reader->pool ));

    do
    {
        unsigned int n = 0;

        LCNCE( lcn_index_reader_term_docs_from( index_reader,
                                                &term_docs,
                                                term,
                                                pool ));

        while( APR_SUCCESS == ( s = lcn_term_docs_next( term_docs ) ))
        {
            LCNCE( lcn_index_reader_delete( index_reader,
                                            lcn_term_docs_doc( term_docs )));
            n++;
        }

        if ( s == LCN_ERR_ITERATOR_NO_NEXT )
        {
            s = APR_SUCCESS;
        }

        *docs_deleted = n;
    }
    while(0);

    if ( NULL != term_docs )
    {
        lcn_term_docs_close( term_docs );
    }

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    if ( NULL != index_reader->log_stream )
    {
        if ( APR_SUCCESS == s )
        {
            fprintf( index_reader->log_stream, "IndexReader %u docs deleted\n", *docs_deleted );
        }
        else
        {
            char buf[1000];
            fprintf( index_reader->log_stream, "IndexReader deleting failed <%d:%s>\n", s, lcn_strerror( s, buf, 1000 ) );
        }
    }

    return s;
}

apr_status_t
lcn_index_reader_delete_documents_by_query( lcn_index_reader_t *index_reader,
                                            lcn_query_t *query,
                                            unsigned int *docs_deleted )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;

    LCNCR( apr_pool_create( &pool, index_reader->pool ));
    *docs_deleted = 0;
    do
    {
        lcn_searcher_t *s;
        lcn_hits_t *hits;
        unsigned int i;
        lcn_bitvector_t *bv;

        lcn_index_reader_null_bitvector( index_reader, &bv, pool );
        lcn_index_searcher_create_by_reader( &s, index_reader, pool );
        lcn_searcher_search( s, &hits, query, NULL, pool );

        for(  i = 0;  i < lcn_hits_length( hits );  i++ )
        {
            lcn_document_t *doc;
            lcn_hits_doc( hits, &doc, i, pool );
            lcn_bitvector_set_bit( bv, lcn_document_id( doc ) );
        }

        lcn_bitvector_not( bv );

        for(  i = 0;  i < lcn_index_reader_max_doc( index_reader );  i++ )
        {
            if ( lcn_bitvector_get_bit( bv, i ))
            {
                (*docs_deleted)++;
                lcn_index_reader_delete( index_reader, i );
            }
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_index_reader_undelete_all( lcn_index_reader_t *index_reader )
{
#if 0
    if(directoryOwner)
      aquireWriteLock();
#endif

    apr_status_t s = index_reader->do_undelete_all( index_reader );

    if ( APR_SUCCESS == s )
    {
        index_reader->is_modified = LCN_TRUE;
    }

    return s;
}

apr_status_t
lcn_index_reader_commit( lcn_index_reader_t *index_reader )
{
    apr_status_t s = APR_SUCCESS;

#if 0
    if(hasChanges){
      if(directoryOwner){
        synchronized (directory) {      // in- & inter-process sync
           new Lock.With(directory.makeLock(IndexWriter.COMMIT_LOCK_NAME),
                   IndexWriter.COMMIT_LOCK_TIMEOUT) {
             public Object doBody() throws IOException {
               doCommit();
               segmentInfos.write(directory);
               return null;
             }
           }.run();
         }
        if (writeLock != null) {
          writeLock.release();  // release write lock
          writeLock = null;
        }
      }
      else
        doCommit();
    }
#endif

    do
    {
        if ( index_reader->has_changes( index_reader ) )
        {
            if ( index_reader->directory_owner )
            {
                LCNCE( index_reader->do_commit( index_reader ) );
                LCNCE( lcn_segment_infos_write( index_reader->segment_infos, index_reader->directory ) );
            }
            else
            {
                LCNCE( index_reader->do_commit( index_reader ) );
            }
        }

        index_reader->is_modified = LCN_FALSE;
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_reader_close( lcn_index_reader_t *index_reader )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_index_reader_commit( index_reader ) );
        LCNCE( index_reader->do_close( index_reader ) );

        if ( index_reader->close_directory )
        {
            LCNCE( lcn_directory_close( index_reader->directory ) );
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_reader_terms( lcn_index_reader_t *index_reader,
                        lcn_term_enum_t **term_enum,
                        apr_pool_t *pool )
{
    return index_reader->terms( index_reader, term_enum, pool );
}

apr_status_t
lcn_index_reader_doc_freq( lcn_index_reader_t *index_reader,
                           const lcn_term_t *term,
                           int *freq )
{
    return index_reader->doc_freq( index_reader, term, freq );
}

apr_status_t
lcn_index_reader_terms_from( lcn_index_reader_t *index_reader,
                             lcn_term_enum_t **term_enum,
                             lcn_term_t *term,
                             apr_pool_t *pool )
{
    apr_status_t s = index_reader->terms_from( index_reader, term_enum, term, pool );

    if ( LCN_ERR_SCAN_ENUM_NO_MATCH == s )
    {
        s = APR_SUCCESS;
    }

    return s;
}

apr_status_t
lcn_index_reader_document( lcn_index_reader_t *index_reader,
                           lcn_document_t **document,
                           unsigned int n,
                           apr_pool_t *pool )
{
    return index_reader->document( index_reader, document, n, pool );
}

lcn_bool_t
lcn_index_reader_is_deleted( lcn_index_reader_t *index_reader,
                             unsigned int n )
{
    return index_reader->is_deleted( index_reader, n );
}

lcn_bool_t
lcn_index_reader_has_norms( lcn_index_reader_t *index_reader,
                            const char *field )
{
    return index_reader->has_norms( index_reader, field );
}

apr_status_t
lcn_index_reader_term_docs( lcn_index_reader_t *index_reader,
                            lcn_term_docs_t **term_docs,
                            apr_pool_t *pool )
{
    return index_reader->term_docs( index_reader, term_docs, pool );
}

apr_status_t
lcn_index_reader_term_docs_from( lcn_index_reader_t *index_reader,
                                 lcn_term_docs_t **term_docs,
                                 const lcn_term_t *term,
                                 apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( index_reader->term_docs( index_reader, term_docs, pool ) );
    s = lcn_term_docs_seek_term( *term_docs, term );

    if( s && ( s != LCN_ERR_SCAN_ENUM_NO_MATCH ) )
    {
        LCNCR( s );
    }
    return s;
}

apr_status_t
lcn_index_reader_set_int_value( lcn_index_reader_t *reader,
                                unsigned int docid,
                                const char *field_name,
                                unsigned int int_value )
{
    reader->is_modified = LCN_TRUE;
    return reader->set_int_value( reader, docid, field_name, int_value );
}

apr_status_t
lcn_index_reader_set_char_value( lcn_index_reader_t *reader,
                                 unsigned int docid,
                                 const char *field_name,
                                 const char *char_value )
{
    reader->is_modified = LCN_TRUE;
    return reader->set_char_value( reader, docid, field_name, char_value );
}

apr_status_t
lcn_index_reader_get_field_infos( lcn_index_reader_t *index_reader,
                                  lcn_list_t *field_infos,
                                  unsigned int field_flags,
                                  unsigned int options_mask )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    do
    {
        LCNCE( apr_pool_create( &cp, index_reader->pool ));
        LCNCE( index_reader->get_field_infos( index_reader, field_infos, field_flags, options_mask ));

        if ( (LCN_FIELD_FIXED_SIZE & options_mask) ==
             (LCN_FIELD_FIXED_SIZE & field_flags ))
        {
            /* add fixed sized fields */

            apr_hash_index_t *hi;

            for( hi = apr_hash_first( cp, index_reader->fs_fields); hi; hi = apr_hash_next( hi ))
            {
                void *vval;
                const void *vkey;
                lcn_fs_field_t *fs;
                lcn_field_info_t *field_info;
                int i;
                lcn_bool_t info_exists = LCN_FALSE;

                apr_hash_this( hi, &vkey, NULL, &vval );
                fs = (lcn_fs_field_t *) vval;

                /* check the name is not yet contained in the field_infos list */

                for( i = 0; i < lcn_list_size( field_infos ); i++ )
                {
                    lcn_field_info_t *finfo = (lcn_field_info_t*) lcn_list_get( field_infos, i );

                    if ( 0 == strcmp( finfo->name, (char*) vkey ))
                    {
                        info_exists = LCN_TRUE;
                        break;
                    }
                }

                if ( ! info_exists )
                {
                    LCNCE( lcn_fs_field_to_field_info( fs, &field_info, lcn_list_pool( field_infos )));
                    LCNCE( lcn_list_add( field_infos, field_info ));
                }
            }
        }
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_index_reader_has_field( lcn_index_reader_t *index_reader,
                            lcn_bool_t *has_field,
                            const char *field_name )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    do
    {
        lcn_list_t *field_infos;
        unsigned int i;

        *has_field = LCN_FALSE;

        LCNCE( apr_pool_create( &cp, index_reader->pool ));
        LCNCE( lcn_list_create( &field_infos, 10, cp ));
        LCNCE( lcn_index_reader_get_field_infos( index_reader,
                                                 field_infos,
                                                 0,
                                                 0  /* given 0, all fields are collected */ ));

        for( i = 0; i < lcn_list_size( field_infos ); i++ )
        {
            lcn_field_info_t *fi = lcn_list_get( field_infos, i );

            if ( 0 == strcmp( field_name, fi->name ))
            {
                *has_field = LCN_TRUE;
                break;
            }
        }
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

lcn_directory_t *
lcn_index_reader_directory( lcn_index_reader_t *index_reader )
{
    return index_reader->directory;
}

apr_status_t
lcn_index_reader_null_bitvector( lcn_index_reader_t *reader,
                                 lcn_bitvector_t **bitvector,
                                 apr_pool_t *pool )
{
    return lcn_null_bitvector_create( bitvector,
                                      lcn_index_reader_max_doc( reader ),
                                      pool );
}

apr_status_t
lcn_index_reader_get_int_field_values( lcn_index_reader_t *reader,
                                       lcn_int_array_t **int_array,
                                       const char *field_name,
                                       int default_val )
{
    apr_status_t s;

    lcn_term_docs_t *term_docs = NULL;
    lcn_term_enum_t *term_enum = NULL;

    do
    {
        lcn_int_array_t *arr;
        char *key;
        apr_pool_t *pool;

        LCNCE( apr_pool_create( &pool, reader->pool ));

        if ( NULL == reader->field_values_cache )
        {
            LCNPV( reader->field_values_cache = apr_hash_make( reader->pool ), APR_ENOMEM );
        }

        key = apr_pstrcat( pool, field_name, "<int>", apr_itoa( pool, default_val), NULL );

        arr = (lcn_int_array_t*) apr_hash_get( reader->field_values_cache,
                                               key,
                                               APR_HASH_KEY_STRING );

        if ( NULL == arr )
        {
            lcn_term_t *term;
            apr_status_t next;
            const char *sort_field_name = lcn_atom_get_str( field_name );
            lcn_fs_field_t *fs_field = NULL;
            unsigned int max_doc = lcn_index_reader_max_doc( reader );

            if ( 0 == default_val )
            {
                LCNCE( lcn_zero_int_array_create( &arr, max_doc, reader->pool ));
            }
            else
            {
                int k;

                LCNCE( lcn_int_array_create( &arr, max_doc, reader->pool ));

                for( k = 0; k < max_doc; k++ )
                {
                    arr->arr[ k ] = default_val;
                }
            }

            /* check if the field is fixed size by looking at */
            /* the fs_fields hash containing fs_fields        */

            fs_field = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                       sort_field_name,
                                                       strlen(sort_field_name) );
            if ( fs_field == NULL )
            {
                LCNCE( lcn_index_reader_term_docs( reader, &term_docs, pool ));
                LCNCE( lcn_term_create ( &term, sort_field_name, "", LCN_TERM_NO_TEXT_COPY, pool ));
                LCNCE( lcn_index_reader_terms_from( reader, &term_enum, term, pool ));

                while( APR_SUCCESS == (next = lcn_term_enum_next( term_enum )))
                {
                    const lcn_term_t *t = lcn_term_enum_term( term_enum );
                    unsigned int term_val;
                    apr_status_t term_docs_next;

                    if ( lcn_term_field( t ) != sort_field_name )
                    {
                        break;
                    }

                    term_val = atoi( lcn_term_text( t ));

                    LCNCE( lcn_term_docs_seek_term_enum( term_docs, term_enum ));

                    while( APR_SUCCESS == ( term_docs_next = lcn_term_docs_next( term_docs ) ))
                    {
                        arr->arr[ lcn_term_docs_doc( term_docs ) ] = term_val;
                    }
                }
            }
            else     /* check if it's a fixed sized field */
            {
                unsigned int i;

                for( i = 0; i < max_doc; i++ )
                {
                    unsigned int ival;
                    LCNCE( lcn_fs_field_int_value( fs_field, &ival, i ));
                    arr->arr[ i ] = ival;
                }
            }
        }

        *int_array = arr;
    }
    while(0);

    if ( NULL != term_docs )
    {
        apr_status_t stat = lcn_term_docs_close( term_docs );
        s = ( APR_SUCCESS == s ? stat : s );
    }

    if ( NULL != term_enum )
    {
        apr_status_t stat = lcn_term_enum_close( term_enum );
        s = ( APR_SUCCESS == s ? stat : s );
    }

    return s;
}

apr_status_t
lcn_index_reader_get_fs_field( lcn_index_reader_t *reader,
                               lcn_fs_field_t **fs_field,
                               const char *field_name )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        const char *sort_field_name = lcn_atom_get_str( field_name );
        *fs_field = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                    sort_field_name,
                                                    strlen(sort_field_name) );
        LCNASSERTM( NULL != *fs_field, LCN_ERR_FIELD_NOT_FOUND, sort_field_name );
    }
    while(0);

    return s;
}


apr_status_t
lcn_index_reader_fs_field_bitvector( lcn_index_reader_t *reader,
                                     lcn_bitvector_t **bitvector,
                                     const char *fname,
                                     lcn_bool_t (*filter_function)( lcn_fs_field_t *, unsigned int ),
                                     apr_pool_t *pool )
{
    return reader->fs_field_bitvector( reader, bitvector, fname, filter_function, pool );
}

apr_status_t
lcn_index_reader_fs_int_field_bitvector( lcn_index_reader_t *reader,
                                         lcn_bitvector_t **bitvector,
                                         const char *fname,
                                         lcn_bool_t (*filter_function) (void* data, unsigned int val, unsigned int doc_order ),
                                         void *filter_data,
                                         apr_pool_t *pool )
{
    return reader->fs_field_int_bitvector( reader, bitvector, fname, filter_function, filter_data, pool );
}

