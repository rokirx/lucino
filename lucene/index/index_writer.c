#include "lcn_index.h"
#include "document_writer.h"
#include "segment_merger.h"
#include "fs_field.h"
#include "compound_file_writer.h"
#include "compound_file_reader.h"
#include "compound_file_util.h"
#include "segment_infos.h"
#include "index_writer_config.h"

char* lcn_index_extensions[] = { "cfs", "fnm", "fdx", "fdt", "tii", "tis", "frq",
    "prx", "del", "tvx", "tvd", "tvf", "tvp" };

void
lcn_index_writer_set_log_stream ( lcn_index_writer_t *index_writer,
                                  FILE *log_stream )
{
    index_writer->log_stream = log_stream;
}

void
lcn_index_writer_set_max_buffered_docs ( lcn_index_writer_t *index_writer,
                                         unsigned int max_buffered_docs )
{
    index_writer->max_buffered_docs = max_buffered_docs;
}

void
lcn_index_writer_set_merge_factor ( lcn_index_writer_t *index_writer,
                                    unsigned int merge_factor )
{
    index_writer->merge_factor = ( merge_factor < 2 ? 2 : merge_factor );
}

static apr_status_t
lcn_index_writer_write_deletable_files ( lcn_index_writer_t *index_writer,
                                         lcn_list_t *files,
                                         apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_ostream_t *output;
        unsigned int i;

        LCNCE( lcn_directory_create_output( index_writer->directory,
                &output,
                "deleteable.new",
                pool ) );

        LCNCE( lcn_ostream_write_int( output, lcn_list_size( files ) ) );

        for ( i = 0; i < lcn_list_size( files ); i++ )
        {
            LCNCE( lcn_ostream_write_string( output, ( char* ) lcn_list_get( files, i ) ) );
        }

        LCNCE( s );

        LCNCE( lcn_ostream_close( output ) );

        LCNCE( lcn_directory_rename_file( index_writer->directory,
                "deleteable.new",
                "deletable" ) );
    }
    while ( 0 );

    return s;
}

static apr_status_t
lcn_index_writer_delete_files ( lcn_index_writer_t *index_writer,
                                lcn_list_t *files_to_delete,
                                lcn_list_t *deletable,
                                apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i;

        for ( i = 0; i < lcn_list_size( files_to_delete ); i++ )
        {
            char *file_name = ( char* ) lcn_list_get( files_to_delete, i );
            apr_status_t stat = lcn_directory_delete_file( index_writer->directory, file_name );
            s = ( s ? s : stat );

            if ( stat )
            {
                lcn_bool_t file_exists;
                apr_status_t st = lcn_directory_file_exists( index_writer->directory,
                        file_name,
                        &file_exists );

                if ( st || file_exists )
                {
                    lcn_list_add( deletable, file_name );
                }
            }
        }
    }
    while ( 0 );

    return s;
}

static apr_status_t
lcn_index_writer_delete_other_files ( lcn_index_writer_t *index_writer,
                                      lcn_list_t *files,
                                      lcn_directory_t *directory,
                                      apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    unsigned int i;
    apr_status_t stat;

    for ( i = 0; i < lcn_list_size( files ); i++ )
    {
        char *file_name = lcn_list_get( files, i );
        stat = lcn_directory_delete_file( directory, file_name );
        s = ( s ? s : stat );
    }

    return s;
}

static apr_status_t
lcn_index_writer_read_deletable_files ( lcn_index_writer_t *index_writer,
                                        lcn_list_t **deletable,
                                        apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_status_t stat;
        lcn_istream_t *input;
        int deletable_size, i;
        lcn_bool_t file_exists;

        LCNCE( lcn_list_create( deletable, 10, pool ) );
        LCNCE( lcn_directory_file_exists( index_writer->directory,
                LCN_INDEX_WRITER_DELETABLE_FILE_NAME,
                &file_exists ) );

        if ( !file_exists )
        {
            break;
        }

        LCNCE( lcn_directory_open_input( index_writer->directory,
                &input,
                LCN_INDEX_WRITER_DELETABLE_FILE_NAME,
                pool ) );

        LCNCE( lcn_istream_read_int( input,
                &deletable_size ) );

        for ( i = deletable_size; i > 0; i-- )
        {
            char *name;
            unsigned int name_len;

            LCNCE( lcn_istream_read_string( input,
                    &name,
                    &name_len,
                    pool ) );

            lcn_list_add( *deletable, name );
        }

        LCNCE( s );

        stat = lcn_istream_close( input );

        s = ( s ? s : stat );
    }
    while ( 0 );

    return s;
}

apr_status_t
lcn_index_writer_delete_segments ( lcn_index_writer_t *index_writer,
                                   lcn_list_t *segments_to_delete )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;

    LCNCR( apr_pool_create( &pool, index_writer->pool ) );

    do
    {
        lcn_list_t *files_to_delete;
        lcn_list_t *deletable;
        unsigned int i;

        LCNCE( lcn_index_writer_read_deletable_files( index_writer,
                &files_to_delete,
                pool ) );

        LCNCE( lcn_list_create( &deletable, 10, pool ) );

        LCNCE( lcn_index_writer_delete_files( index_writer,
                files_to_delete,
                deletable,
                pool ) );

        for ( i = 0; i < lcn_list_size( segments_to_delete ); i++ )
        {
            lcn_index_reader_t *reader = ( lcn_index_reader_t* ) lcn_list_get( segments_to_delete, i );
            lcn_list_t *files;

            LCNCE( lcn_segment_reader_files( reader, &files, pool ) );

            if ( lcn_index_reader_directory( reader ) == index_writer->directory )
            {
                LCNCE( lcn_index_writer_delete_files( index_writer,
                        files,
                        deletable,
                        pool ) );

            }
            else
            {
                LCNCE( lcn_index_writer_delete_other_files( index_writer,
                        files,
                        lcn_index_reader_directory( reader ),
                        pool ) );
            }
        }

        LCNCE( s );

        LCNCE( lcn_index_writer_write_deletable_files( index_writer, deletable, pool ) );
    }
    while ( 0 );

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

lcn_similarity_t *
lcn_index_writer_get_similarity ( lcn_index_writer_t *index_writer )
{
    return index_writer->similarity;
}

static apr_status_t
lcn_index_writer_merge_segments_impl ( lcn_index_writer_t *index_writer,
                                       unsigned int min_segment,
                                       unsigned int end )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;

    do
    {
        char *segment_name;
        lcn_segment_merger_t *segment_merger;
        lcn_list_t *segments_to_delete;
        int i;
        unsigned int merged_doc_count;

        LCNCE( apr_pool_create( &pool, index_writer->pool ) );

        LCNCE( lcn_segment_infos_get_next_name( index_writer->segment_infos,
                &segment_name,
                index_writer->seg_name_subpool ) );

#if 0
        //TODO:
        if ( infoStream != null ) infoStream.print( "merging segments" );
#endif
        LCNCE( lcn_segment_merger_create( &segment_merger,
                index_writer,
                segment_name,
                pool ) );

        LCNCE( lcn_list_create( &segments_to_delete, 10, pool ) );

        for ( i = min_segment; i < end; i++ )
        {
            lcn_segment_info_t *si;
            lcn_index_reader_t *reader;

            LCNCE( lcn_segment_infos_get( index_writer->segment_infos, &si, i ) );
            LCNCE( lcn_segment_reader_create_by_info( &reader, si, pool ) );

#if 0
            // TODO
            //if (infoStream != null)
            //   infoStream.print(" " + si.name + " (" + si.docCount + " docs)");
#endif
            LCNCE( lcn_segment_merger_add_reader( segment_merger, reader ) );

            if ( lcn_index_reader_directory( reader ) == index_writer->directory || /* if we own the directory */
                    lcn_index_reader_directory( reader ) == index_writer->ram_directory )
            {
                /* queue segment for deletion */
                LCNCE( lcn_list_add( segments_to_delete, reader ) );
            }
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_segment_merger_merge( segment_merger, &merged_doc_count ) );

        /* remove old infos & add new */

        for ( i = end - 1; i >= ( int ) min_segment; i-- )
        {
            lcn_segment_infos_remove( index_writer->segment_infos, i );
        }

        LCNCE( lcn_segment_infos_add_info( index_writer->segment_infos,
                index_writer->directory,
                segment_name,
                merged_doc_count ) );

        /* close readers before we attempt to delete non-obsolete segments */

        LCNCE( lcn_segment_merger_close_readers( segment_merger ) );

        /* commit before deleting */
        LCNCE( lcn_segment_infos_write( index_writer->segment_infos,
                index_writer->directory ) );

        /* delete now-unused segments */
        LCNCE( lcn_index_writer_delete_segments( index_writer, segments_to_delete ) );

#if 0
        if ( infoStream != null )
        {
            infoStream.println( " into " + mergedName + " (" + mergedDocCount + " docs)" );
        }
#endif

    }
    while ( 0 );

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

static apr_status_t
lcn_index_writer_merge_segments ( lcn_index_writer_t *index_writer, unsigned int min_segment )
{
    return lcn_index_writer_merge_segments_impl( index_writer,
            min_segment,
            lcn_segment_infos_size( index_writer->segment_infos ) );
}

static lcn_bool_t
check_dir_owner ( lcn_segment_infos_t *segment_infos,
                  lcn_directory_t *directory )
{
    apr_status_t s;
    lcn_segment_info_t *info;

    do
    {
        LCNCE( lcn_segment_infos_get( segment_infos, &info,
                lcn_segment_infos_size( segment_infos ) - 1 ) );
    }
    while ( 0 );

    if ( s )
    {
        return LCN_FALSE;
    }

    return lcn_segment_info_directory( info ) == directory;
}

static apr_status_t
lcn_index_writer_flush_ram_segments ( lcn_index_writer_t *index_writer )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        int min_segment = ( int ) lcn_segment_infos_size( index_writer->segment_infos ) - 1;
        int doc_count = 0;
        lcn_segment_info_t *info;

        while ( min_segment >= 0 )
        {
            LCNCE( lcn_segment_infos_get( index_writer->segment_infos, &info, min_segment ) );

            if ( lcn_segment_info_directory( info ) != index_writer->ram_directory )
            {
                break;
            }

            doc_count += lcn_segment_info_doc_count( info );

            min_segment--;
        }

        if ( s )
        {
            break;
        }

        if ( min_segment < 0 || /* add one FS segment? */
                ( doc_count + lcn_segment_info_doc_count( info ) ) > index_writer->merge_factor ||
                !( check_dir_owner( index_writer->segment_infos, index_writer->ram_directory ) ) )
        {
            min_segment++;
        }

        if ( min_segment >= lcn_segment_infos_size( index_writer->segment_infos ) )
        {
            return APR_SUCCESS; /* none to merge */
        }

        LCNCE( lcn_index_writer_merge_segments( index_writer, min_segment ) );
    }
    while ( 0 );

    return s;
}

static apr_status_t
lcn_index_writer_maybe_merge_segments ( lcn_index_writer_t *index_writer )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        apr_int64_t target_merge_docs = ( apr_int64_t ) index_writer->max_buffered_docs;

        while ( target_merge_docs <= ( apr_int64_t ) index_writer->max_merge_docs )
        {
            /* find segments smaller than current target size */
            int min_segment = ( int ) lcn_segment_infos_size( index_writer->segment_infos );
            unsigned int merge_docs = 0;

            while ( --min_segment >= 0 )
            {
                lcn_segment_info_t *si;

                LCNCE( lcn_segment_infos_get( index_writer->segment_infos, &si, min_segment ) );

                if ( si->doc_count >= target_merge_docs )
                {
                    break;
                }

                merge_docs += si->doc_count;
            }

            if ( s )
            {
                break;
            }

            if ( merge_docs >= ( int ) target_merge_docs )
            {
                LCNCE( lcn_index_writer_merge_segments( index_writer, min_segment + 1 ) );
            }
            else
            {
                break;
            }

            target_merge_docs *= index_writer->merge_factor;
        }
    }
    while ( 0 );

    return s;
}

/**
 * @Deprecated Die Methode ist veraltet. Die neue Methode lcn_index_writer_create_impl_neu
 * sollte jetzt genutzt werden. 
 */
static apr_status_t
lcn_index_writer_create_impl ( lcn_index_writer_t **index_writer,
                               lcn_directory_t *directory,
                               lcn_bool_t create,
                               lcn_bool_t close_dir,
                               apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *si_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &si_pool, pool ) );

        LCNPV( *index_writer = ( lcn_index_writer_t* ) apr_pcalloc( pool, sizeof (lcn_index_writer_t ) ),
                APR_ENOMEM );

        ( *index_writer )->pool = pool;

        LCNCE( apr_pool_create( &( ( *index_writer )->seg_name_subpool ), pool ) );
        LCNCE( apr_pool_create( &( ( *index_writer )->ram_dir_subpool ), pool ) );
        LCNCE( lcn_ram_directory_create( &( ( *index_writer )->ram_directory ), ( *index_writer )->ram_dir_subpool ) );
        LCNCE( lcn_segment_infos_create( &( ( *index_writer )->segment_infos ), si_pool ) );
        LCNCE( lcn_default_similarity_create( &( ( *index_writer )->similarity ), pool ) );

        ( *index_writer )->close_dir = close_dir;
        ( *index_writer )->directory = directory;

        ( *index_writer )->max_field_length = LCN_INDEX_WRITER_DEFAULT_MAX_FIELD_LENGTH;
        ( *index_writer )->term_index_interval = LCN_INDEX_WRITER_DEFAULT_TERM_INDEX_INTERVAL;
        ( *index_writer )->max_buffered_docs = LCN_INDEX_WRITER_DEFAULT_MAX_BUFFERED_DOCS;
        ( *index_writer )->max_merge_docs = LCN_INDEX_WRITER_DEFAULT_MAX_MERGE_DOCS;
        ( *index_writer )->merge_factor = LCN_INDEX_WRITER_DEFAULT_MERGE_FACTOR;

        if ( create )
        {
            LCNCE( lcn_segment_infos_write( ( *index_writer )->segment_infos, directory ) );
        }
        else
        {
            LCNCE( lcn_segment_infos_read( ( *index_writer )->segment_infos, directory ) );
        }

        {
            unsigned int i;

            for ( i = 0; i < lcn_segment_infos_size( ( *index_writer )->segment_infos ); i++ )
            {
                lcn_segment_info_t *segment_info;

                LCNCE( lcn_segment_infos_get( ( *index_writer )->segment_infos,
                        &segment_info,
                        i ) );
                ( *index_writer )->docs_count += lcn_segment_info_doc_count( segment_info );
            }
        }

        /* init fixed sized fields */

        LCNPV( ( *index_writer )->fs_fields = apr_hash_make( pool ), APR_ENOMEM );
        LCNCE( lcn_directory_fs_field_read_field_infos( ( *index_writer )->fs_fields, directory, pool ) );
    }
    while ( 0 );

    if ( s && ( NULL != si_pool ) )
    {
        apr_pool_destroy( si_pool );
    }

    return s;
}

static apr_status_t
lcn_index_writer_create_impl_neu ( lcn_index_writer_t **index_writer,
                                   lcn_directory_t *directory,
                                   lcn_index_writer_config_t *iwc,
                                   apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *cp;

    do
    {
        lcn_bool_t create = LCN_FALSE;
        unsigned int open_mode = lcn_index_writer_config_get_open_mode( iwc );

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNPV( *index_writer = lcn_object_create( lcn_index_writer_t, pool ), APR_ENOMEM );

        (*index_writer)->pool = pool;

        lcn_index_writer_set_config( *index_writer, iwc );

        if ( open_mode == LCN_INDEX_WRITER_CONFIG_OPEN_MODE_CREATE )
        {
            create = LCN_TRUE;
        }
        else if ( open_mode == LCN_INDEX_WRITER_CONFIG_OPEN_MODE_APPEND )
        {
            create = LCN_FALSE;
        }
        else
        {
            lcn_bool_t exists;

            LCNCE( lcn_directory_reader_index_exists( directory, &exists, cp ));
            /*
             * CREATE_OR_APPEND - create only if an index does not exist
             * create = !DirectoryReader.indexExists(directory);
             */
        }
        /*  
         * If index is too old, reading the segments will throw
         * IndexFormatTooOldException.
         *segmentInfos = new SegmentInfos();
         */
    }
    while ( 0 );


    return s;
}

apr_status_t
lcn_index_writer_create_by_config ( lcn_index_writer_t **index_writer,
                                    lcn_directory_t *directory,
                                    lcn_index_writer_config_t *iwc,
                                    apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    LCNCR( lcn_index_writer_create_impl_neu( index_writer,
                                             directory,
                                             iwc,
                                             pool ) );

    return s;
}

apr_status_t
lcn_index_writer_create_by_directory ( lcn_index_writer_t **index_writer,
                                       lcn_directory_t *directory,
                                       lcn_bool_t create,
                                       apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_index_writer_create_impl( index_writer, directory, create, LCN_FALSE, pool ) );

    return s;
}

apr_status_t
lcn_index_writer_create_by_path ( lcn_index_writer_t **index_writer,
                                  const char *path,
                                  lcn_bool_t create,
                                  apr_pool_t *pool )
{
    apr_status_t s;
    lcn_directory_t *directory;
    apr_pool_t *dir_pool = NULL;

    do
    {
        LCNCE( apr_pool_create( &dir_pool, pool ) );
        LCNCE( lcn_fs_directory_create( &directory, path, create, dir_pool ) );
        LCNCE( lcn_index_writer_create_impl( index_writer, directory, create, LCN_TRUE, pool ) );
    }
    while ( 0 );

    if ( s && ( dir_pool != NULL ) )
    {
        apr_pool_destroy( dir_pool );
    }

    return s;
}

apr_status_t
lcn_index_writer_write_binary_to_stream ( FILE* stream,
                                          const char* varname,
                                          const char* data,
                                          unsigned int size )
{
    apr_status_t s;
    apr_status_t i;

    LCNASSERTR( 0 < fprintf( stream, "char %s[%u] = { ", varname, size ), LCN_ERR_IO );

    for ( i = 0; i < size; i++ )
    {
        LCNASSERTR( 0 < fprintf( stream, "%u, ", ( unsigned int ) ( unsigned char ) data[i] ),
                LCN_ERR_IO );
    }

    LCNASSERTR( 0 < fprintf( stream, " }\n" ), LCN_ERR_IO );

    return s;
}

#define LCN_INDEX_WRITER_WRITE_PROP( is_fun, prop ) \
LCNASSERTR( 0 <= fprintf( stream, "%s", is_fun( field ) ? #prop " " : "" ), LCN_ERR_IO );

apr_status_t
lcn_index_writer_write_dump_entry ( FILE* stream,
                                    lcn_list_t *fields_list,
                                    apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t* cp;
    unsigned int i;

    LCNCR( apr_pool_create( &cp, pool ) );
    LCNASSERTR( 0 < fprintf( stream, "lcn_document fields=%u\n", lcn_list_size( fields_list ) ), LCN_ERR_IO );


    for ( i = 0; i < lcn_list_size( fields_list ); i++ )
    {
        char* str;
        lcn_field_t *field = ( lcn_field_t* ) lcn_list_get( fields_list, i );
        const char *fval = lcn_field_value( field );

        apr_pool_clear( cp );

        LCNCE( lcn_string_escape( &str,
                lcn_field_name( field ),
                "\"",
                '\\',
                cp ) );

        LCNASSERTR( 0 < fprintf( stream, "lcn_field=\"%s\"\n", str ), LCN_ERR_IO );
        LCNASSERTR( 0 < fprintf( stream, "properties=" ), LCN_ERR_IO );

        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_is_binary, BINARY );
        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_is_indexed, INDEXED );
        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_is_tokenized, TOKENIZED );
        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_is_stored, STORED );
        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_is_fixed_size, FIXED_SIZE );
        LCN_INDEX_WRITER_WRITE_PROP( lcn_field_omit_norms, OMIT_NORMS );

        LCNASSERTR( 0 < fprintf( stream, "\n" ), LCN_ERR_IO );

        if ( lcn_field_is_fixed_size( field ) )
        {
            unsigned int size = lcn_field_size( field );
            unsigned int bytes = ( size >> 3 ) + ( ( size & 7 ) ? 1 : 0 );

            LCNASSERTR( 0 < fprintf( stream, "size=%u\n", lcn_field_size( field ) ), LCN_ERR_IO );

            LCNCE( lcn_index_writer_write_binary_to_stream( stream,
                    "bin_value",
                    fval,
                    bytes ) );

            if ( NULL != lcn_field_default_value( field ) )
            {
                LCNCE( lcn_index_writer_write_binary_to_stream( stream,
                        "default_value",
                        lcn_field_default_value( field ),
                        bytes ) );
            }
        }

        if ( lcn_field_is_tokenized( field ) )
        {
            lcn_analyzer_t* an;
            apr_status_t stat = lcn_field_get_analyzer( field, &an );

            LCNASSERTR( fprintf( stream,
                    "lcn_analyzer=%s\n",
                    stat == APR_SUCCESS ? lcn_analyzer_type( an ) : "NONE" ),
                    LCN_ERR_IO );
        }
        else
        {
            LCNASSERTR( fprintf( stream, "lcn_analyzer=NONE\n" ), LCN_ERR_IO );
        }

        if ( !lcn_field_is_fixed_size( field ) )
        {
            if ( lcn_field_is_binary( field ) )
            {
                LCNCE( lcn_index_writer_write_binary_to_stream( stream,
                        "bin_value",
                        fval,
                        lcn_field_size( field ) ) );
            }
            else
            {
                char* e_val;

                LCNCE( lcn_string_escape( &e_val, fval, "\"", '\\', cp ) );
                LCNASSERTR( fprintf( stream, "value=\"%s\"\n", e_val ), LCN_ERR_IO );
            }
        }

        LCNASSERTR( fprintf( stream, "\n" ), LCN_ERR_IO );
    }

    apr_pool_destroy( cp );

    return s;
}

unsigned int
lcn_index_writer_write_fixed_sized_fields ( lcn_index_writer_t *index_writer,
                                            lcn_document_t *document )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    do
    {
        unsigned int i;
        lcn_list_t *flist = lcn_document_get_fields( document );

        LCNCE( apr_pool_create( &cp, index_writer->pool ) );

        for ( i = 0; i < lcn_list_size( flist ); i++ )
        {
            lcn_field_t *doc_field = lcn_list_get( flist, i );

            if ( lcn_field_is_fixed_size( doc_field ) )
            {
                lcn_directory_fs_field_t *field;
                const char *fname = lcn_field_name( doc_field );

                field = ( lcn_directory_fs_field_t* ) apr_hash_get( index_writer->fs_fields, fname, strlen( fname ) );

                if ( NULL == field )
                {
                    /* first check for existing field files */

                    char *file_name;
                    lcn_bool_t file_exists;

                    LCNPV( file_name = apr_pstrcat( cp, fname, ".fsf", NULL ), APR_ENOMEM );
                    LCNCE( lcn_directory_file_exists( index_writer->directory,
                            file_name,
                            &file_exists ) );

                    if ( file_exists )
                    {
                        lcn_istream_t *is;

                        LCNCE( lcn_directory_open_input( index_writer->directory, &is, file_name, cp ) );
                        LCNCE( lcn_directory_fs_field_read( &field, fname, is, index_writer->pool ) );
                        LCNCE( lcn_istream_close( is ) );
                    }
                    else
                    {
                        lcn_fs_field_t *f;

                        LCNCE( lcn_directory_fs_field_create( &f,
                                fname,
                                0, /* docs_count    */
                                lcn_field_size( doc_field ),
                                NULL, /* directory */
                                index_writer->pool ) );
                        field = ( lcn_directory_fs_field_t* ) f;

                        if ( NULL != lcn_field_default_value( doc_field ) )
                        {
                            LCNCE( lcn_directory_fs_field_set_default_value( field, lcn_field_default_value( doc_field ) ) );
                        }
                    }

                    apr_hash_set( index_writer->fs_fields, fname, strlen( fname ), field );
                }

                LCNASSERT( lcn_fs_field_data_size( ( lcn_fs_field_t* ) field ) == lcn_field_size( doc_field ),
                        LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );

                {
                    int i;
                    int bytes = ( lcn_fs_field_data_size( ( lcn_fs_field_t* ) field ) >> 3 ) +
                            ( lcn_fs_field_data_size( ( lcn_fs_field_t* ) field ) % 8 ? 1 : 0 );
                    const char *d1 = lcn_fs_field_default_val( ( lcn_fs_field_t* ) field );
                    const char *d2 = lcn_field_default_value( doc_field );

                    for ( i = 0; i < bytes; i++ )
                    {
                        if ( d1[i] != d2[i] )
                        {
                            LCNCE( LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );
                        }
                    }
                }

                LCNCE( s );

                LCNCE( lcn_fs_field_set_value( ( lcn_fs_field_t* ) field, lcn_field_value( doc_field ), index_writer->docs_count ) );
            }
        }
    }
    while ( 0 );

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_index_writer_add_document ( lcn_index_writer_t *index_writer,
                                lcn_document_t *document )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;
    lcn_list_t *flist = lcn_document_get_fields( document );

#if 0
    fprintf( stderr, "IC <%10d/%10d>\n",
            apr_pool_num_bytes( index_writer->pool, 0 ),
            apr_pool_num_bytes( index_writer->pool, 1 )
            - apr_pool_num_bytes( index_writer->pool, 0 )
            - apr_pool_num_bytes( index_writer->ram_dir_subpool, 1 )
            - apr_pool_num_bytes( index_writer->seg_name_subpool, 1 )
            - apr_pool_num_bytes( index_writer->segment_infos->pool, 1 )
            - apr_pool_num_bytes( index_writer->directory->pool, 1 ) );

    fprintf( stderr, "IR <%10d/%10d>\n",
            apr_pool_num_bytes( index_writer->ram_dir_subpool, 0 ),
            apr_pool_num_bytes( index_writer->ram_dir_subpool, 1 )
            - apr_pool_num_bytes( index_writer->ram_dir_subpool, 0 ) );

    fprintf( stderr, "IS <%10d/%10d>\n",
            apr_pool_num_bytes( index_writer->seg_name_subpool, 0 ),
            apr_pool_num_bytes( index_writer->seg_name_subpool, 1 )
            - apr_pool_num_bytes( index_writer->seg_name_subpool, 0 ) );

    fprintf( stderr, "ID <%10d/%10d>\n",
            apr_pool_num_bytes( index_writer->directory->pool, 0 ),
            apr_pool_num_bytes( index_writer->directory->pool, 1 )
            - apr_pool_num_bytes( index_writer->directory->pool, 0 ) );

    fprintf( stderr, "II <%10d/%10d>\n",
            apr_pool_num_bytes( index_writer->segment_infos->pool, 0 ),
            apr_pool_num_bytes( index_writer->segment_infos->pool, 1 )
            - apr_pool_num_bytes( index_writer->segment_infos->pool, 0 ) );
#endif

    LCNASSERTR( 0 < lcn_list_size( flist ), LCN_ERR_INDEX_WRITER_ADDING_EMTY_DOCUMENT );

    if ( NULL != index_writer->log_stream )
    {
        LCNCR( lcn_index_writer_write_dump_entry( index_writer->log_stream,
                flist,
                pool ) );
    }

    LCNCR( apr_pool_create( &pool, index_writer->pool ) );

    do
    {
        lcn_document_writer_t *document_writer;
        char *segment_name;

        LCNCE( lcn_index_writer_write_fixed_sized_fields( index_writer, document ) );

        LCNCE( lcn_document_writer_create( &document_writer,
                index_writer->ram_directory,
                index_writer,
                pool ) );

        if ( 0 == ( index_writer->seg_name_counter++ / 1000 ) )
        {
            index_writer->seg_name_counter = 0;
            apr_pool_clear( index_writer->seg_name_subpool );
        }

        LCNCE( lcn_segment_infos_get_next_name( index_writer->segment_infos,
                &segment_name,
                index_writer->seg_name_subpool ) );

        LCNCE( lcn_document_writer_add_document( document_writer, segment_name, document ) );

        LCNCE( lcn_segment_infos_add_info( index_writer->segment_infos,
                index_writer->ram_directory,
                segment_name,
                1 ) );

        LCNCE( lcn_index_writer_maybe_merge_segments( index_writer ) );

        index_writer->docs_count++;
    }
    while ( 0 );

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

apr_status_t
lcn_index_writer_flush_fixed_size_fields ( lcn_index_writer_t *index_writer )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    if ( 0 == apr_hash_count( index_writer->fs_fields ) )
    {
        return APR_SUCCESS;
    }

    do
    {
        apr_hash_index_t *hi;

        LCNCE( apr_pool_create( &cp, index_writer->pool ) );

        for ( hi = apr_hash_first( cp, index_writer->fs_fields ); hi; hi = apr_hash_next( hi ) )
        {
            lcn_directory_fs_field_t *field;
            const void *vkey;
            void *vval;

            apr_hash_this( hi, &vkey, NULL, &vval );

            /* TODO: can we always assume lcn_directory_fs_field_t ?  */

            field = ( lcn_directory_fs_field_t * ) vval;
            field->directory = index_writer->directory;


            if ( field->parent.docs_count < index_writer->docs_count )
            {
                LCNCE( lcn_fs_field_set_value( ( lcn_fs_field_t* ) field,
                        lcn_fs_field_default_val( ( const lcn_fs_field_t * ) field ),
                        index_writer->docs_count - 1 ) );
            }

            LCNASSERT( field->parent.docs_count == index_writer->docs_count, LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET );

            LCNCE( lcn_fs_field_commit( ( lcn_fs_field_t* ) field, cp ) );
#if 0

            LCNCE( lcn_fs_field_update_fields_def( field, index_writer->directory, cp ) );
#endif
        }
    }
    while ( 0 );

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_index_writer_close ( lcn_index_writer_t *index_writer )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_index_writer_flush_fixed_size_fields( index_writer ) );
        LCNCE( lcn_index_writer_flush_ram_segments( index_writer ) );
        LCNCE( lcn_directory_close( index_writer->ram_directory ) );
        LCNCE( lcn_fs_field_close_fields_in_hash( index_writer->fs_fields ) );

#if 0
        if ( writeLock != null )
        {
            writeLock.release( ); // release write lock
            writeLock = null;
        }
        if ( closeDir )
            directory.close( );
#endif

    }
    while ( 0 );

    return s;
}

unsigned int
lcn_index_writer_get_max_field_length ( lcn_index_writer_t *index_writer )
{
    return index_writer->max_field_length;
}

void
lcn_index_writer_set_max_field_length ( lcn_index_writer_t *index_writer,
                                        unsigned int max_field_length )
{
    index_writer->max_field_length = max_field_length;
}

unsigned int
lcn_index_writer_get_term_index_interval ( lcn_index_writer_t *index_writer )
{
    return index_writer->term_index_interval;
}

void
lcn_index_writer_set_term_index_interval ( lcn_index_writer_t *index_writer,
                                           unsigned int term_index_interval )
{
    index_writer->term_index_interval = term_index_interval;
}

void
lcn_index_writer_set_config ( lcn_index_writer_t *index_writer,
                              lcn_index_writer_config_t *iwc )
{
    index_writer->iwc = iwc;
}

apr_status_t
lcn_index_writer_optimize ( lcn_index_writer_t *index_writer )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_index_writer_flush_ram_segments( index_writer ) );

        while ( 1 )
        {
            int min_segment;

            if ( lcn_segment_infos_size( index_writer->segment_infos ) < 1 )
            {
                break;
            }

            if ( lcn_segment_infos_size( index_writer->segment_infos ) == 1 )
            {
                lcn_segment_info_t *si;
                lcn_bool_t has_deletions;
                
                LCNCE( lcn_segment_infos_get( index_writer->segment_infos, &si, 0 ) );
                LCNCE( lcn_segment_info_has_deletions( si, &has_deletions ) );
                
                if (( ! has_deletions &&
                     lcn_segment_info_directory( si ) == index_writer->directory ) )
                {
                    break;
                }
            }

            min_segment = lcn_segment_infos_size( index_writer->segment_infos ) -
                    index_writer->merge_factor;

            LCNCE( lcn_index_writer_merge_segments( index_writer, min_segment < 0 ? 0 : min_segment ) );
        }
    }
    while ( 0 );

    return s;
}

/**
 * Erzeugt zuerst einen optimierten Index, welcher als Basis für den compound file Index genutzt wird.
 */
apr_status_t
lcn_index_writer_cf_optimize ( lcn_index_writer_t *index_writer )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *child_pool = NULL;
    int i = 0;

    do
    {
        char *csf_filename = NULL;
        lcn_compound_file_writer_t *cfw = NULL;
        lcn_list_t *index_files = NULL;
        lcn_segment_info_t *segment_info = NULL;
        lcn_list_t *files_to_delete = NULL;


        LCNCE( lcn_index_writer_optimize( index_writer ) );

        LCNCE( apr_pool_create( &child_pool, index_writer->pool ));

        /**
         * optimize index erzeugt eine Index, welcher aus mehreren Datein besteht.
         * Diese m¸ssen nun in ein cp-File ¸berf¸hrt werden.
         */

        int segment_size = lcn_segment_infos_size( index_writer->segment_infos );

        if ( segment_size > 1 )
        {
            LCNLOG( "After lcn_index_writer_optimize segment size < 1" );
            return LCN_ERR_INDEX_OUT_OF_RANGE;
        }

        LCNPV( segment_info = lcn_list_get( index_writer->segment_infos->list, 0 ), LCN_ERR_NULL_PTR );

        LCNPV( csf_filename = apr_pstrcat( child_pool, segment_info->name, ".cfs", NULL ), APR_ENOMEM );

        LCNCE( lcn_compound_file_writer_create( &cfw, index_writer->directory, csf_filename,  child_pool ) );

        LCNCE( lcn_directory_list( index_writer->directory, &index_files, child_pool ) );

        LCNCE( lcn_list_create( &files_to_delete, CP_EXT_COUNT, child_pool ) );

        /**
         * Zuerst werden alle basic Dateien hinzugef√ºgt. (fnm, frq, tii ...)
         * Diese Dateien sind nach dem optimze Prozess vorhanden.
         */
        for ( i = 0; i < CP_EXT_COUNT; i++ )
        {
            char *filename;
            LCNPV( filename = apr_pstrcat( index_writer->pool,
                                           segment_info->name,
                                           ".", COMPOUND_EXTENSIONS[i], NULL),
                                           LCN_ERR_NULL_PTR );

            LCNCE( lcn_compound_file_writer_add_file( cfw, filename) );
            lcn_list_add( files_to_delete, filename );
        }
        LCNCE(s);

        /**
         * Hier werden alle vorhandenen Normen hinzugef√ºgt. (segment_name.f<digit>)
         */
        for ( i = 0; i < lcn_list_size( index_files ); i++ )
        {
            char *marker = NULL;
            char *filename = lcn_list_get( index_files, i );
            if ( ( marker = strstr( filename, ".f" ) ) && marker != NULL )
            {
                marker += 2;
                if ( lcn_string_is_digit( marker ) )
                {
                    LCNCE( lcn_compound_file_writer_add_file( cfw, filename ) );
                    lcn_list_add( files_to_delete, filename );
                }
            }
        }
        LCNCE(s);

#if 0
          // Field norm files
    for (int i = 0; i < fieldInfos.size(); i++) {
      FieldInfo fi = fieldInfos.fieldInfo(i);
      if (fi.isIndexed && !fi.omitNorms) {
        files.add(segment + ".f" + i);
      }
    }

#endif

        LCNCE( lcn_compound_file_writer_close( cfw ) );

        lcn_directory_delete_files( index_writer->directory, files_to_delete );

    }
    while ( 0 );


    if ( child_pool != NULL )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}

apr_status_t
lcn_index_writer_add_indexes ( lcn_index_writer_t *index_writer,
                               lcn_list_t *dirs )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *pool = NULL;

    do
    {
        unsigned int start, i, j;

        LCNCE( apr_pool_create( &pool, index_writer->pool ) );
        LCNCE( lcn_index_writer_optimize( index_writer ) );

        /* merge fixed fields first */
        LCNCE( lcn_fs_field_merge_indexes( index_writer->directory, dirs, pool ) );

        /* re-init fields */
        LCNPV( index_writer->fs_fields = apr_hash_make( index_writer->pool ), APR_ENOMEM );
        LCNCE( lcn_directory_fs_field_read_field_infos( index_writer->fs_fields, index_writer->directory, index_writer->pool ) );

        start = lcn_segment_infos_size( index_writer->segment_infos );


        for ( i = 0; i < lcn_list_size( dirs ); i++ )
        {
            lcn_segment_infos_t *sis;
            lcn_directory_t *dir = ( lcn_directory_t* ) lcn_list_get( dirs, i );

            LCNCE( lcn_segment_infos_create( &sis, pool ) );
            LCNCE( lcn_segment_infos_read( sis, dir ) );

            for ( j = 0; j < lcn_segment_infos_size( sis ); j++ )
            {
                lcn_segment_info_t *si;

                LCNCE( lcn_segment_infos_get( sis, &si, j ) );
                LCNCE( lcn_segment_infos_add_info( index_writer->segment_infos,
                        lcn_segment_info_directory( si ),
                        lcn_segment_info_name( si ),
                        lcn_segment_info_doc_count( si ) ) );

                index_writer->docs_count += lcn_segment_info_doc_count( si );
            }

            LCNCE( s );

            /* merge newly added segments in log(n) passes */

            while ( lcn_segment_infos_size( index_writer->segment_infos ) >
                    start + index_writer->merge_factor )
            {
                unsigned int base;

                for ( base = start;
                        base < lcn_segment_infos_size( index_writer->segment_infos );
                        base++ )
                {
                    unsigned int end = lcn_segment_infos_size( index_writer->segment_infos );
                    end = ( end < base + index_writer->merge_factor ) ? end : base + index_writer->merge_factor;
                    if ( end - base > 1 )
                    {
                        LCNCE( lcn_index_writer_merge_segments_impl( index_writer, base, end ) );
                    }
                }

                LCNCE( s );
            }

            LCNCE( s );
        }

        LCNCE( s );

        LCNCE( lcn_index_writer_optimize( index_writer ) );

    }
    while ( 0 );

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

apr_status_t
lcn_index_writer_delete_if_empty ( lcn_index_writer_t *index_writer )
{
    apr_status_t s;
    apr_pool_t *pool = NULL;

    do
    {
        lcn_list_t *file_list;

        LCNCE( apr_pool_create( &pool, index_writer->pool ) );
        LCNCE( lcn_directory_list( index_writer->directory,
                &file_list,
                pool ) );

        if ( 1 == lcn_list_size( file_list ) &&
                0 == strcmp( "segments", ( char* ) lcn_list_get( file_list, 0 ) ) )
        {
            LCNCE( lcn_directory_delete_file( index_writer->directory, "segments" ) );
            LCNCE( lcn_directory_remove( index_writer->directory ) );
        }
    }
    while ( 0 );

    return s;
}

apr_status_t
lcn_index_writer_create_index_by_dump ( const char* index_path,
                                        const char* dump_file,
                                        apr_hash_t* analyzer_map,
                                        lcn_bool_t optimize,
                                        apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        char *dump;
        unsigned int length;
        lcn_document_dump_iterator_t* iterator;
        lcn_index_writer_t *index_writer;
        lcn_document_t *doc;

        LCNCE( lcn_string_from_file( &dump, dump_file, &length, pool ) );
        LCNCE( lcn_document_dump_iterator_create( &iterator, dump, analyzer_map, pool ) );
        LCNCE( lcn_index_writer_create_by_path( &index_writer, index_path, LCN_TRUE, pool ) );

        while ( APR_SUCCESS == lcn_document_dump_iterator_next( iterator, &doc, pool ) )
        {
            LCNCE( lcn_index_writer_add_document( index_writer, doc ) );
        }

        LCNCE( lcn_index_writer_close( index_writer ) );
        LCNCE( optimize ? lcn_index_writer_optimize( index_writer ) : APR_SUCCESS );
    }
    while ( 0 );

    return APR_SUCCESS;
}

unsigned int
lcn_index_writer_max_doc ( lcn_index_writer_t *index_writer )
{
    unsigned int count = 0;

#if 0
    if ( docWriter != null )
        count = docWriter.getNumDocsInRAM( );
    else
        count = 0;

    for ( int i = 0; i < segmentInfos.size( ); i++ )
        count += segmentInfos.info( i ).docCount;
    return count;
#endif

    return count;
}

