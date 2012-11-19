#include "index_reader.h"
#include "index_writer.h"
#include "term_enum.h"
#include "fs_field.h"
#include "document.h"
#include "compound_file_reader.h"

/****************************************************************************************
 *
 *                           Norm implementation
 *
 ****************************************************************************************/

struct lcn_norm_t {

    lcn_index_input_t *istream;
    lcn_byte_array_t *bytes;
    lcn_bool_t dirty;
    int number;
};

static apr_status_t
lcn_norm_create( lcn_norm_t **norm,
                 lcn_index_input_t *istream,
                 int number,
                 apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *norm = (lcn_norm_t*) apr_pcalloc(pool, sizeof(lcn_norm_t) ),
               APR_ENOMEM );

        (*norm)->istream = istream;
        (*norm)->number  = number;
    }
    while(0);

    return s;
}

#if 0
  private class Norm {

    private void reWrite() throws IOException {
      // NOTE: norms are re-written in regular directory, not cfs
      IndexOutput out = directory().createOutput(segment + ".tmp");
      try {
        out.writeBytes(bytes, maxDoc());
      } finally {
        out.close();
      }
      String fileName;
      if(cfsReader == null)
          fileName = segment + ".f" + number;
      else{
          // use a different file name if we have compound format
          fileName = segment + ".s" + number;
      }
      directory().renameFile(segment + ".tmp", fileName);
      this.dirty = false;
    }
  }
#endif

apr_status_t
lcn_segment_reader_files( lcn_index_reader_t *reader,
                          lcn_list_t **files,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_status_t i;

        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) reader;

        LCNCE( lcn_list_create( files, 16, pool ));

        for ( i = 0; i < LCN_INDEX_EXTENSIONS_SIZE; i++ )
        {
            lcn_bool_t file_exists;
            char *name = apr_pstrcat( pool, segment_reader->segment, ".",
                                      lcn_index_extensions[i], NULL );

            LCNCE( lcn_directory_file_exists( segment_reader->parent.directory,
                                              name,
                                              &file_exists ));

            if ( file_exists )
            {
                lcn_list_add( *files, name );
            }
        }

        for ( i = 0; i < lcn_field_infos_size( segment_reader->field_infos ); i++ )
        {
            lcn_field_info_t *fi;

            LCNCE( lcn_field_infos_by_number( segment_reader->field_infos,
                                              &fi,
                                              i ));

            if ( lcn_field_info_is_indexed( fi ) &&
                 ! lcn_field_info_omit_norms( fi ) )
            {
                lcn_bool_t file_exists;
                char *name = apr_pstrcat( pool, segment_reader->segment,
                                          ".f", apr_itoa( pool, i ), NULL );

                LCNCE( lcn_directory_file_exists( segment_reader->parent.directory,
                                                  name,
                                                  &file_exists ));

                if( file_exists )
                {
                    lcn_list_add( *files, name );
                }
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_reader_get_field_infos( lcn_index_reader_t *index_reader,
                                    lcn_list_t *field_infos_list,
                                    unsigned int field_options,
                                    unsigned int options_mask )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        unsigned int size = lcn_field_infos_size( segment_reader->field_infos );
        unsigned int i;

        for( i = 0; i < size; i++ )
        {
            lcn_field_info_t *field_info;

            LCNCE( lcn_field_infos_by_number( segment_reader->field_infos,
                                              &field_info,
                                              i ) );

            if ( (field_info->field_bits & options_mask) ==
                 (field_options & options_mask ) )
            {
                unsigned int n;
                lcn_bool_t field_is_in_list = LCN_FALSE;

                for( n = 0; n < lcn_list_size( field_infos_list ); n++ )
                {
                    lcn_field_info_t *fi = lcn_list_get( field_infos_list, n );

                    if ( 0 == strcmp( field_info->name, fi->name ) )
                    {
                        /* TODO ? some less restrictive condition? */

                        /* Accept merging of indexed and not indexed fields */

                        if ( ( field_info->field_bits & ~LCN_FIELD_INDEXED ) !=
                             ( fi->field_bits & ~LCN_FIELD_INDEXED) )
                        {
                            LCNCM(LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION, field_info->name );
                        }

                        /* in case that the the indexed property is different          */
                        /* assert that the resulting info has set the indexed property */

                        if ( field_info->field_bits & LCN_FIELD_INDEXED )
                        {
                            fi->field_bits |= LCN_FIELD_INDEXED;
                        }

                        field_is_in_list = LCN_TRUE;
                        break;
                    }
                }

                if ( s )
                {
                    break;
                }

                if ( ! field_is_in_list )
                {
                    lcn_field_info_t *new_info;

                    LCNPV( new_info = (lcn_field_info_t*)
                           apr_pcalloc( lcn_list_pool( field_infos_list ), sizeof(lcn_field_info_t) ),
                           APR_ENOMEM );

                    *new_info = *field_info;

                    LCNCE( lcn_list_add( field_infos_list, new_info ));
                }
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_reader_init_stream( lcn_segment_reader_t *segment_reader,
                                lcn_index_input_t **stream,
                                const char *segment_name,
                                const char *extension )
{
    apr_status_t s;

    do
    {
        char *file_name;
        lcn_bool_t file_exists;

        LCNPV( file_name = apr_pstrcat( segment_reader->parent.pool, segment_name, extension, NULL ),
               APR_ENOMEM );

        LCNCE( lcn_directory_file_exists( segment_reader->parent.directory, file_name, &file_exists ));

        if ( file_exists )
        {
            LCNCE( lcn_directory_open_input( segment_reader->parent.directory,
                                             stream,
                                             file_name,
                                             segment_reader->parent.pool ));
        }
        else
        {
            *stream = NULL;
        }
    }
    while(0);

    return s;
}

/**
 * It is the only function handling getting norms.
 */
static apr_status_t
lcn_segment_reader_open_norms( lcn_segment_reader_t *segment_reader )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;

    do
    {
        unsigned int size = lcn_field_infos_size( segment_reader->field_infos );
        unsigned int i;

        LCNCE( apr_pool_create( &pool, segment_reader->parent.pool ));

        for ( i = 0; i < size; i++ )
        {
            lcn_field_info_t *field_info;

            LCNCE( lcn_field_infos_by_number( segment_reader->field_infos, &field_info, i ) );

            if ( lcn_field_info_is_indexed( field_info ) &&
                 ! lcn_field_info_omit_norms( field_info ) )
            {
                lcn_norm_t *norm;
                lcn_index_input_t *istream;
                char *file_name = NULL;
                char *ext = apr_itoa( pool, field_info->number );

                LCNPV( ext, APR_ENOMEM );
                LCNPV( file_name = apr_pstrcat( pool, segment_reader->segment, ".f", ext, NULL ), APR_ENOMEM );
#if 0
                // look first if there are separate norms in compound format
                String fileName = segment + ".s" + fi.number;
                Directory d = directory();
                if(!d.fileExists(fileName)){
                    fileName = segment + ".f" + fi.number;
                    d = cfsDir;
            }
#endif
                LCNCE( lcn_directory_open_input( segment_reader->parent.directory, &istream, file_name, segment_reader->parent.pool ) );
                LCNCE( lcn_norm_create( &norm, istream, field_info->number, segment_reader->parent.pool ) );

                apr_hash_set( segment_reader->parent.norms, field_info->name, strlen(field_info->name), norm );
            }
        }
    }
    while(0);

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

static apr_status_t
lcn_segment_reader_initialize( lcn_segment_reader_t *segment_reader,
                               lcn_segment_info_t *segment_info,
                               apr_pool_t* pool)
{
    apr_status_t s;
    apr_pool_t* child_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &child_pool, pool ));

        lcn_bool_t has_deletions;
        lcn_bool_t cf_segment_exists = LCN_FALSE;
        char* cf_segment_file_name= NULL;

        segment_reader->segment = segment_info->name;
        LCNPV( cf_segment_file_name = apr_pstrcat( child_pool,
                                                   segment_reader->segment,
                                                   ".cfs", NULL ), APR_ENOMEM );

        LCNCE(lcn_directory_file_exists(segment_reader->parent.directory,
                                        cf_segment_file_name,
                                        &cf_segment_exists ));

        if ( cf_segment_exists == LCN_TRUE )
        {
            lcn_directory_t *cf_dir = NULL;
            LCNCE(lcn_cfs_directory_create( &cf_dir,
                                            segment_reader->parent.directory,
                                            cf_segment_file_name,
                                            child_pool) );

            segment_reader->parent.directory = cf_dir;
        }

#if 0
        TODO Compound file support
            // Use compound file directory for some files, if it exists
            Directory cfsDir = directory();
        if (directory().fileExists(segment + ".cfs")) {
            cfsReader = new CompoundFileReader(directory(), segment + ".cfs");
            cfsDir = cfsReader;
        }
#endif
        LCNCE( lcn_field_infos_create_from_dir( &(segment_reader->field_infos),
                                                segment_reader->parent.directory,
                                                segment_reader->segment,
                                                segment_reader->parent.pool ) );

        LCNCE( lcn_fields_reader_create( &(segment_reader->fields_reader),
                                         segment_reader->parent.directory,
                                         segment_reader->segment,
                                         segment_reader->field_infos,
                                         segment_reader->parent.pool ));

        /* LCN_ERR_RAM_FILE_NOT_FOUND */

        s =  lcn_term_infos_reader_create( &(segment_reader->tis),
                                           segment_reader->parent.directory,
                                           segment_reader->segment,
                                           segment_reader->field_infos,
                                           segment_reader->parent.pool );

        /* it is possible to create Lucene-indexes without term infos */

        if ( s == LCN_ERR_RAM_FILE_NOT_FOUND || s == LCN_ERR_TERM_INFOS_READER_NO_TIS_FILE )
        {
            segment_reader->tis = NULL;
            s = APR_SUCCESS;
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_segment_info_has_deletions( segment_info, &has_deletions ) );

        if( has_deletions )
        {
            char *bv_name;

            LCNPV( bv_name = apr_pstrcat( segment_reader->parent.pool, segment_info->name, ".del", NULL ),
                   APR_ENOMEM );

            LCNCE( lcn_bitvector_from_dir( &(segment_reader->deleted_docs),
                                           segment_reader->parent.directory,
                                           bv_name,
                                           segment_reader->parent.pool ));
        }

        /* it is possible to create indexes without proximities */

        LCNCE( lcn_segment_reader_init_stream( segment_reader, &(segment_reader->prox_stream),
                                               segment_info->name, ".prx" ));

        /* it is possible to create indexes without frequencies */

        LCNCE( lcn_segment_reader_init_stream( segment_reader, &(segment_reader->freq_stream),
                                               segment_info->name, ".frq" ));

        LCNCE( lcn_segment_reader_open_norms( segment_reader ) );
#if 0
        if (fieldInfos.hasVectors()) { // open term vector files only as needed
            termVectorsReaderOrig = new TermVectorsReader(cfsDir, segment, fieldInfos);
        }
#endif
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_reader_term_docs( lcn_index_reader_t *index_reader,
                              lcn_term_docs_t **term_docs,
                              apr_pool_t *pool )
{
    return lcn_segment_term_docs_create( index_reader, term_docs, pool );
}

static apr_status_t
lcn_segment_reader_doc_freq( lcn_index_reader_t *index_reader,
                             const lcn_term_t *term,
                             int *freq )
{
    apr_status_t s;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        lcn_term_info_t *term_info;

        s = lcn_term_infos_reader_get_by_term( segment_reader->tis, &term_info, term );

        if ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ||
             LCN_ERR_SCAN_ENUM_NO_MATCH == s )
        {
            *freq = 0;
            s = APR_SUCCESS;
        }
        else if ( APR_SUCCESS == s )
        {
            *freq = term_info->doc_freq;
        }
    }
    while(0);

    return s;
}

static lcn_bool_t
lcn_segment_reader_is_deleted( lcn_index_reader_t *index_reader,
                               unsigned int n )
{
    lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

    return ( NULL != segment_reader->deleted_docs &&
             lcn_bitvector_get_bit( segment_reader->deleted_docs, n ) );
}

static apr_status_t /* synchronized */
lcn_segment_reader_document( lcn_index_reader_t *index_reader,
                             lcn_document_t **document,
                             unsigned int n,
                             apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        LCNASSERT( !lcn_segment_reader_is_deleted( index_reader, n ), LCN_ERR_ACCESS_DELETED_DOCUMENT );
        LCNCE( lcn_fields_reader_doc( segment_reader->fields_reader, document, n, pool ) );

        if ( index_reader->fs_fields && 0 != apr_hash_count( index_reader->fs_fields ))
        {
            LCNCE( lcn_index_reader_add_fs_fields( index_reader, *document, n, pool ));
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_reader_terms_from( lcn_index_reader_t *index_reader,
                               lcn_term_enum_t **term_enum,
                               lcn_term_t *term,
                               apr_pool_t *pool )
{
    lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

    apr_status_t s = lcn_term_infos_reader_terms_from( segment_reader->tis, term_enum, term, pool );

    if ( APR_SUCCESS == s || LCN_ERR_SCAN_ENUM_NO_MATCH == s )
    {
        (*term_enum)->skip_first_next = LCN_TRUE;
    }
    else if (  LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s )
    {
        s = APR_SUCCESS;
    }

    return s;
}

static apr_status_t
lcn_segment_reader_terms( lcn_index_reader_t *index_reader,
                          lcn_term_enum_t **term_enum,
                          apr_pool_t *pool )
{
    return lcn_term_infos_reader_terms( ((lcn_segment_reader_t*)index_reader)->tis, term_enum, pool );
}

static lcn_bool_t
lcn_segment_reader_has_norms( lcn_index_reader_t *index_reader,
                              const char *field )
{
    return NULL != apr_hash_get( index_reader->norms ,
                                 field,
                                 strlen(field) );
}

static apr_status_t
lcn_segment_reader_close_norms( lcn_index_reader_t *index_reader )
{
#if 0
    synchronized (norms);
#endif

    apr_status_t s = APR_SUCCESS;
    apr_hash_index_t *hi;
    apr_pool_t *pool;

    if ( NULL == index_reader->norms )
    {
        return s;
    }

    if (( s = apr_pool_create( &pool, index_reader->pool )))
    {
        return s;
    }

    do
    {
        void *val;
        lcn_norm_t *norm;
        apr_status_t status;

        for ( hi = apr_hash_first( pool, index_reader->norms );
              hi;
              hi = apr_hash_next( hi ) )
        {
            apr_hash_this( hi, NULL, NULL, &val );
            norm = (lcn_norm_t*) val;
            status = lcn_index_input_close( norm->istream );

            /* on error try to close all norms */

            if ( status )
            {
                s = status;
            }
        }
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

static apr_status_t
lcn_segment_reader_do_close( lcn_index_reader_t *index_reader )
{
    apr_status_t s;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        LCNCE( lcn_fields_reader_close( segment_reader->fields_reader ) );

        if ( NULL != segment_reader->tis )
        {
            LCNCE( lcn_term_infos_reader_close( segment_reader->tis ) );
        }

        if ( NULL != segment_reader->freq_stream )
        {
            LCNCE( lcn_index_input_close( segment_reader->freq_stream ) );
        }

        if ( NULL != segment_reader->prox_stream )
        {
            LCNCE( lcn_index_input_close( segment_reader->prox_stream ) );
        }

        LCNCE( lcn_segment_reader_close_norms( index_reader ) );

        /* close fixed sized fields */

        LCNCR( lcn_fs_field_close_fields_in_hash( index_reader->fs_fields ));

#if 0
        if (termVectorsReaderOrig != null)
            termVectorsReaderOrig.close();

#endif

    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_reader_do_commit( lcn_index_reader_t *index_reader )
{
    apr_status_t s;
    apr_pool_t *pool;

    if (( s = apr_pool_create( &pool, index_reader->pool ) ))
    {
        return s;
    }

    do
    {
        char *tmp_file;
        char *del_file;

        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        LCNPV( tmp_file = apr_pstrcat( pool, segment_reader->segment, ".tmp", NULL ), APR_ENOMEM );
        LCNPV( del_file = apr_pstrcat( pool, segment_reader->segment, ".del", NULL ), APR_ENOMEM );

        if ( segment_reader->deleted_docs_dirty )
        {
            LCNCE( lcn_bitvector_write( segment_reader->deleted_docs,
                                        segment_reader->parent.directory,
                                        tmp_file,
                                        pool ));
            LCNCE( lcn_directory_rename_file( segment_reader->parent.directory,
                                              tmp_file, del_file ) );
        }

        if ( segment_reader->undelete_all )
        {
            lcn_bool_t file_exists;

            LCNCE( lcn_directory_file_exists( segment_reader->parent.directory,
                                              del_file, &file_exists ));

            if ( file_exists )
            {
                LCNCE( lcn_directory_delete_file( segment_reader->parent.directory, del_file ) );
            }
        }

#if 0

        if (normsDirty) {               // re-write norms
            Enumeration values = norms.elements();
            while (values.hasMoreElements()) {
                Norm norm = (Norm) values.nextElement();
                if (norm.dirty) {
                    norm.reWrite();
                }
            }
        }
#endif
        segment_reader->deleted_docs_dirty = LCN_FALSE;
        /* NOT implemented yet: segment_reader->norms_dirty = LCN_FALSE; */
        segment_reader->undelete_all = LCN_FALSE;

        /* commit possibly modified fixed sized fields */

        {
            apr_hash_index_t *hi;

            for( hi = apr_hash_first( pool, index_reader->fs_fields); hi; hi = apr_hash_next( hi ))
            {
                lcn_fs_field_t *fs_field;
                void *vval;

                apr_hash_this( hi, NULL, NULL, &vval );
                fs_field = (lcn_fs_field_t *) vval;

                if ( fs_field->is_modified )
                {
                    LCNCE( lcn_fs_field_commit( fs_field, pool ));
                }
            }
        }
    }
    while(0);

    apr_pool_destroy( pool );

    return s;
}

static apr_status_t
lcn_segment_reader_do_undelete_all( lcn_index_reader_t *index_reader )
{
    lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

    segment_reader->deleted_docs = NULL;

    if ( NULL != segment_reader->del_vector_pool )
    {
        apr_pool_clear( segment_reader->del_vector_pool );
    }

    segment_reader->deleted_docs_dirty = LCN_FALSE;
    segment_reader->undelete_all = LCN_TRUE;

    return APR_SUCCESS;
}

static unsigned int
lcn_segment_reader_max_doc( lcn_index_reader_t *index_reader )
{
    return lcn_fields_reader_size( ((lcn_segment_reader_t*)index_reader)->fields_reader );
}

static apr_status_t
lcn_segment_reader_do_delete( lcn_index_reader_t *index_reader, int num_doc )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        if ( NULL == segment_reader->deleted_docs )
        {
            if ( NULL == segment_reader->del_vector_pool )
            {
                apr_pool_t *pool;

                LCNCE( apr_pool_create( &pool, segment_reader->parent.pool ));

                 segment_reader->del_vector_pool = pool;
            }

            LCNCE( lcn_bitvector_create ( &(segment_reader->deleted_docs),
                                          lcn_segment_reader_max_doc( index_reader ),
                                          segment_reader->del_vector_pool ));
        }

        segment_reader->deleted_docs_dirty = LCN_TRUE;
        segment_reader->undelete_all = LCN_FALSE;

        lcn_bitvector_set_bit( segment_reader->deleted_docs, num_doc );
    }
    while(0);

    return s;
}

static lcn_bool_t
lcn_segment_reader_has_deletions( lcn_index_reader_t *index_reader )
{
    return NULL != ((lcn_segment_reader_t*)index_reader)->deleted_docs;
}

static unsigned int
lcn_segment_reader_num_docs( lcn_index_reader_t *index_reader )
{
    lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;
    unsigned int num_docs = lcn_fields_reader_size( segment_reader->fields_reader );

    if ( NULL != segment_reader->deleted_docs )
    {
        num_docs -= lcn_bitvector_count( segment_reader->deleted_docs );
    }

    return num_docs;
}

static apr_status_t
lcn_segment_reader_read_norms_to_array( lcn_index_reader_t *index_reader,
                                        lcn_byte_array_t* result_norms,
                                        unsigned int offset,
                                        const char *field )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *p = NULL;
    lcn_index_input_t *norms_stream = NULL;

    do
    {
        lcn_norm_t *norm = (lcn_norm_t*) apr_hash_get( index_reader->norms,
                                                       field,
                                                       APR_HASH_KEY_STRING );
        if ( NULL == norm )
        {
            s = LCN_ERR_NORMS_NOT_FOUND;
            break;
        }

        if ( NULL != norm->bytes )  /* can copy from cache */
        {
            memcpy( result_norms->arr + offset, norm->bytes->arr, norm->bytes->length );
        }
        else
        {
            unsigned int max_doc = lcn_index_reader_max_doc( index_reader );

            LCNCE( apr_pool_create( &p, index_reader->pool ) );
            LCNCE( lcn_index_input_clone( norm->istream, &norms_stream, p ) );
            LCNCE( lcn_index_input_seek( norms_stream, 0 ) );
            LCNCE( lcn_index_input_read_bytes( norms_stream, result_norms->arr + offset, 0, &max_doc ) );
            LCNCE( lcn_index_input_close( norms_stream ) );
        }
    }
    while(0);

    if ( NULL != norms_stream )
    {
        apr_status_t stat = lcn_index_input_close( norms_stream );
        s = (s ? s : stat );
    }

    if ( NULL != p )
    {
        apr_pool_destroy( p );
    }

    return s;
}

static apr_status_t
lcn_segment_reader_norms( lcn_index_reader_t *index_reader,
                          lcn_byte_array_t** result_norms,
                          const char *field )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *p = NULL;

    do
    {
        lcn_norm_t *norm = (lcn_norm_t*) apr_hash_get( index_reader->norms,
                                                       field,
                                                       APR_HASH_KEY_STRING );

        if ( NULL == norm )
        {
            s = LCN_ERR_NORMS_NOT_FOUND;
            break;
        }

        if ( NULL == norm->bytes )
        {
            lcn_byte_array_t *new_norm_bytes;
            unsigned int max_doc = lcn_index_reader_max_doc( index_reader );

            LCNCE( lcn_byte_array_create( &new_norm_bytes,
                                          max_doc,
                                          index_reader->pool ));
            LCNCE( lcn_segment_reader_read_norms_to_array( index_reader,
                                                           new_norm_bytes,
                                                           0,
                                                           field ) );
            norm->bytes = new_norm_bytes;
        }

        *result_norms = norm->bytes;
    }
    while(0);

    if ( NULL != p )
    {
        apr_pool_destroy( p );
    }

    return s;
}

static apr_status_t
lcn_segment_reader_fs_field_bitvector( lcn_index_reader_t *index_reader,
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

        lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) apr_hash_get( index_reader->fs_fields,
                                                                                    fname,
                                                                                    APR_HASH_KEY_STRING );

        LCNCE( lcn_index_reader_null_bitvector( index_reader, bitvector, pool ) );

        if ( NULL == field )
        {
            return s;
        }

        /* assert that the field is loaded to RAM */

        LCNCE( lcn_directory_fs_field_init_buffer( field ));

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
lcn_segment_reader_fs_field_int_bitvector( lcn_index_reader_t *reader,
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

        lcn_directory_fs_field_t *field = (lcn_directory_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                                                    fname,
                                                                                    APR_HASH_KEY_STRING );

        LCNCE( lcn_index_reader_null_bitvector( reader, bitvector, pool ) );

        if ( NULL == field )
        {
            return s;
        }

        /* assert that the field is loaded to RAM */

        LCNCE( lcn_directory_fs_field_init_buffer( field ));

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

static apr_status_t
lcn_segment_reader_set_int_value ( lcn_index_reader_t *reader,
                                   unsigned int docid,
                                   const char *fname,
                                   unsigned int int_value )
{
    apr_status_t s;
    lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                        fname,
                                                        strlen(fname) );
    if ( NULL != f )
    {
        LCNCR( lcn_fs_field_set_int_value( f, int_value, docid ) );

        reader->is_modified = LCN_TRUE;
        f     ->is_modified = LCN_TRUE;

        return s;
    }

    return LCN_ERR_FIELD_NOT_FOUND;

}

static apr_status_t
lcn_segment_reader_set_char_value ( lcn_index_reader_t *reader,
                                    unsigned int docid,
                                    const char *fname,
                                    const char *char_value )
{
    apr_status_t s;
    lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                        fname,
                                                        strlen(fname) );
    if ( NULL != f )
    {
        LCNCR( lcn_fs_field_set_value( f, char_value, docid ) );

        reader->is_modified = LCN_TRUE;
        f     ->is_modified = LCN_TRUE;

        return s;
    }

    return LCN_ERR_FIELD_NOT_FOUND;

}


static apr_status_t
lcn_segment_reader_add_fs_field_def( lcn_index_reader_t *reader,
                                     lcn_field_t *field )
{
    apr_status_t s = APR_SUCCESS;
    lcn_fs_field_t *f = (lcn_fs_field_t*) apr_hash_get( reader->fs_fields,
                                                        lcn_field_name( field ),
                                                        strlen(lcn_field_name( field ) ));
    do
    {
        lcn_fs_field_t *fs_field;

        LCNCE( lcn_directory_fs_field_create( &fs_field,
                                              lcn_field_name( field ),
                                              0,
                                              lcn_field_size( field ),
                                              reader->directory,
                                              reader->pool ));
        if ( NULL == f )
        {
            LCNCE( lcn_directory_fs_field_set_default_value( (lcn_directory_fs_field_t *) fs_field,
                                                             lcn_field_default_value( field )));
            fs_field->is_modified = LCN_TRUE;

            apr_hash_set( reader->fs_fields,
                          apr_pstrdup( apr_hash_pool_get( reader->fs_fields ), lcn_fs_field_name( fs_field ) ),
                          strlen(lcn_fs_field_name( fs_field )),
                          fs_field );
        }
        else
        {
            LCNASSERT( lcn_fs_field_info_is_equal( fs_field, f ), LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );
        }
    }
    while(0);

    return s;
}

static lcn_bool_t
lcn_segment_reader_has_changes( lcn_index_reader_t *index_reader )
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

apr_status_t
lcn_segment_reader_create( lcn_index_reader_t **index_reader,
                           lcn_directory_t *directory,
                           lcn_segment_infos_t *segment_infos,
                           lcn_segment_info_t *segment_info,
                           lcn_bool_t close_directory,
                           lcn_bool_t own_dir,
                           apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_segment_reader_t *reader = lcn_object_create( lcn_segment_reader_t, pool );

        LCNCE( lcn_index_reader_init( lcn_cast_index_reader( reader ), directory,
                                      segment_infos,
                                      close_directory,
                                      own_dir, pool ) );

        LCNCE( lcn_segment_reader_initialize( reader, segment_info, pool ) );

        reader->parent.num_docs        = lcn_segment_reader_num_docs;
        reader->parent.max_doc         = lcn_segment_reader_max_doc;
        reader->parent.doc_freq        = lcn_segment_reader_doc_freq;
        reader->parent.has_deletions   = lcn_segment_reader_has_deletions;
        reader->parent.do_delete       = lcn_segment_reader_do_delete;
        reader->parent.do_undelete_all = lcn_segment_reader_do_undelete_all;
        reader->parent.do_close        = lcn_segment_reader_do_close;
        reader->parent.do_commit       = lcn_segment_reader_do_commit;
        reader->parent.terms           = lcn_segment_reader_terms;
        reader->parent.terms_from      = lcn_segment_reader_terms_from;
        reader->parent.document        = lcn_segment_reader_document;
        reader->parent.is_deleted      = lcn_segment_reader_is_deleted;
        reader->parent.has_norms       = lcn_segment_reader_has_norms;
        reader->parent.term_docs       = lcn_segment_reader_term_docs;
        reader->parent.term_positions  = lcn_segment_term_positions_create;
        reader->parent.get_field_infos = lcn_segment_reader_get_field_infos;
        reader->parent.get_norms       = lcn_segment_reader_norms;

        reader->parent.read_norms_to_array    = lcn_segment_reader_read_norms_to_array;
        reader->parent.fs_field_bitvector     = lcn_segment_reader_fs_field_bitvector;
        reader->parent.fs_field_int_bitvector = lcn_segment_reader_fs_field_int_bitvector;
        reader->parent.set_int_value          = lcn_segment_reader_set_int_value;
        reader->parent.set_char_value         = lcn_segment_reader_set_char_value;
        reader->parent.add_fs_field_def       = lcn_segment_reader_add_fs_field_def;
        reader->parent.has_changes            = lcn_segment_reader_has_changes;

        *index_reader = lcn_cast_index_reader( reader );
    }
    while(0);

    return s;
}
