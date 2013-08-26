#include "segment_infos.h"
#include "directory.h"
#include "index_file_names.h"
#include "index_input.h"
#include "io_context.h"
#include "lcn_store.h"
#include "lcn_error_codes.h"
#include "lucene.h"
#include "../codecs/codec.h"

/********************************************************
 *                                                      *
 * Functions of segment_infos                           *
 *                                                      *
 * read current version not implemented. is a conv.     *
 * function                                             *
 *                                                      *
 ********************************************************/

/**
 * NEXT TESTEN !!
 */
apr_status_t
lcn_segment_infos_generation_from_segments_file_name( char* filename, apr_int64_t* generation )
{
    apr_status_t s = APR_SUCCESS;

    if ( 0 == strcmp( filename, LCN_INDEX_FILE_NAMES_SEGMENTS ) )
    {
        *generation = 0;
    }
    else if( lcn_string_starts_with( filename, LCN_INDEX_FILE_NAMES_SEGMENTS ) )
    {
        filename += (strlen(LCN_INDEX_FILE_NAMES_SEGMENTS) + 1);
        *generation = apr_atoi64( filename );
    }
    else
    {
        s = LCN_ERR_INVALID_ARGUMENT;
    }

    return s;
}

apr_status_t
lcn_segment_infos_create( lcn_segment_infos_t **segment_infos,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *segment_infos = apr_pcalloc( pool, sizeof(lcn_segment_infos_t)), APR_ENOMEM );
        LCNCE( apr_pool_create( &((*segment_infos)->subpool), pool ) );
        LCNCE( lcn_list_create( &((*segment_infos)->list), 10, (*segment_infos)->subpool ) );
        LCNCE( lcn_list_create( &((*segment_infos)->segments), 25, (*segment_infos)->subpool ) );
        (*segment_infos)->user_data = apr_hash_make( (*segment_infos)->subpool );

        (*segment_infos)->pool = pool;
    }
    while(0);

    return s;
}

void
lcn_segment_infos_remove( lcn_segment_infos_t *segment_infos, unsigned int i )
{
    lcn_list_remove( segment_infos->list, i );
}

unsigned int
lcn_segment_infos_size( lcn_segment_infos_t *segment_infos )
{
    //return segment_infos ? lcn_list_size( segment_infos->list ) : 0;
    return lcn_list_size( segment_infos->list );
}

apr_uint64_t
lcn_segment_infos_version( lcn_segment_infos_t *segment_infos )
{
    return segment_infos->version;
}

int
lcn_segment_infos_format( lcn_segment_infos_t *segment_infos )
{
    return segment_infos->format;
}

apr_status_t
lcn_segment_infos_get_next_name( lcn_segment_infos_t *segment_infos,
                                 char **seg_name,
                                 apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        char buf[10];
        buf[0] = '_';
        lcn_itoa36( segment_infos->counter, buf + 1 );
        LCNPV( *seg_name = apr_pstrdup( pool, buf ), APR_ENOMEM );
        segment_infos->counter++;
    }
    while(0);

    return s;
}

apr_status_t
lcn_segment_infos_get( lcn_segment_infos_t *segment_infos,
                       lcn_segment_info_per_commit_t **segment_info,
                       unsigned int nth )
{
    *segment_info = lcn_list_get( segment_infos->list, nth );
    return APR_SUCCESS;
}

apr_status_t
lcn_segment_infos_add_info ( lcn_segment_infos_t *segment_infos,
                             lcn_directory_t *directory,
                             const char *name,
                             unsigned int count )
{
    apr_status_t s;

    do
    {
        apr_pool_t *pool = lcn_list_pool( segment_infos->list );

        lcn_segment_info_per_commit_t *segment_info_pc = lcn_object_create( lcn_segment_info_per_commit_t, pool );
        segment_info_pc->segment_info = lcn_object_create( lcn_segment_info_t, pool );

        LCNCE( lcn_list_add( segment_infos->list, segment_info_pc ) );
        LCNPV( segment_info_pc->segment_info->name = apr_pstrdup( pool, name ), APR_ENOMEM );

        segment_info_pc->segment_info->doc_count = count;
        segment_info_pc->segment_info->directory = directory;

        if ( 0 == (segment_infos->counter % 1000))
        {
            unsigned int lsize = lcn_list_size( segment_infos->list );
            unsigned int i;
            lcn_list_t *list;
            apr_pool_t *p;

            LCNCE( apr_pool_create( &p, segment_infos->pool ));
            LCNCE( lcn_list_create( &list, 100, p ));

            for (i = 0; i < lsize; i++ )
            {
                lcn_segment_info_per_commit_t *si_pc = (lcn_segment_info_per_commit_t*) lcn_list_get( segment_infos->list, i );
                lcn_segment_info_per_commit_t *new_si_pc = lcn_object_create( lcn_segment_info_per_commit_t, p );
                new_si_pc->segment_info = lcn_object_create( lcn_segment_info_t, p );

                LCNCE( lcn_list_add( list, new_si_pc ));
                LCNPV( new_si_pc->segment_info->name = apr_pstrdup( p, si_pc->segment_info->name ), APR_ENOMEM );

                new_si_pc->segment_info->doc_count = si_pc->segment_info->doc_count;
                new_si_pc->segment_info->directory = si_pc->segment_info->directory;
            }

            if ( s )
            {
                break;
            }

            apr_pool_destroy( segment_infos->subpool );
            segment_infos->subpool = p;
            segment_infos->list = list;
        }
    }
    while(0);

    return s;
}

/**
 * Get the next segments_N filename that will be written.
 */
static char*
get_next_segment_file_name( lcn_segment_infos_t *segement_infos, apr_pool_t *pool )
{
    unsigned int next_generation;

    if( segement_infos->generation == -1 )
    {
        next_generation = 1;
    }
    else
    {
        next_generation = segement_infos->generation + 1;
    }

    return lcn_index_file_names_file_name_from_generation( LCN_INDEX_FILE_NAMES_SEGMENTS,
                                                           "",
                                                           next_generation,
                                                           pool );
}

apr_status_t
lcn_segment_infos_write( lcn_segment_infos_t *segment_infos,
                         lcn_directory_t *dir )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;
    lcn_index_output_t *segn_file = NULL;
    lcn_index_output_t *segn_output = NULL;

    do
    {
        unsigned int size = lcn_segment_infos_size( segment_infos ), i = 0;
        char* seg_file_name = NULL;

        LCNCE( apr_pool_create( &cp, dir->pool ));

        seg_file_name = get_next_segment_file_name( segment_infos, cp );

        // Always advance the generation on write:
        if( segment_infos->generation == -1 )
        {
            segment_infos->generation = 1;
        }
        else
        {
            segment_infos->generation++;
        }

        LCNCE( lcn_directory_create_output( dir, &segn_file, seg_file_name, segment_infos->pool ) );
        LCNCE( lcn_checksum_index_output_create( &segn_output, segn_file, segment_infos->pool ) );

        LCNCE( lcn_codec_util_write_header( segn_output, "segments", LCN_SEGMENT_INFOS_VERSION_40 ) );
        LCNCE( lcn_index_output_write_long( segn_output, segment_infos->version));
        LCNCE( lcn_index_output_write_int( segn_output, segment_infos->counter ));
        LCNCE( lcn_index_output_write_int( segn_output, size ));

        for( i = 0; i < size; i++ )
        {
            lcn_segment_info_t *info;
            lcn_segment_info_per_commit_t *info_pc;

            LCNCE( lcn_segment_infos_get( segment_infos, &info_pc, i ) );
            info = lcn_segment_info_per_commit_info( info_pc );

            LCNCE( lcn_index_output_write_string( segn_output, info->name ) );
#if 0
            TODO: implement Codec

            segnOutput.writeString(si.getCodec().getName());
#endif
            LCNCE( lcn_index_output_write_long( segn_output, info_pc->del_gen ) );
            LCNCE( lcn_index_output_write_int( segn_output, info_pc->del_count ) );

#if 0
            TODO: implement assert

            assert si.dir == directory;
            assert siPerCommit.getDelCount() <= si.getDocCount();
#endif
        }

#if 0
        segnOutput.writeStringStringMap(userData);
#endif

        LCNCE( lcn_checksum_index_output_write_string_string_hash( segn_output, segment_infos->user_data ) );
        segment_infos->pending_seqn_output = segn_output;

        if ( s )
        {
            break;
        }

#if 0
        TODO: implement first step userData-Hash-Map is empty

        segnOutput.writeStringStringMap(userData);
        pendingSegnOutput = segnOutput;
#endif
   }
   while(0);

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_segment_infos_has_separate_norms( lcn_segment_info_t *segment_info,
                                      lcn_bool_t *flag,
                                      apr_pool_t *pool)
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *child_pool;
    *flag = LCN_FALSE;

    do
    {
        lcn_list_t *file_list;
        char *pattern;
        unsigned int i, pattern_len;

        LCNCE( apr_pool_create( &child_pool, pool ));
        LCNCE( lcn_directory_list(segment_info->directory, &file_list, child_pool) );
        LCNPV( pattern = apr_pstrcat(child_pool, segment_info->name, ".s", NULL ), LCN_ERR_NULL_PTR);

        pattern_len = strlen(pattern);

        for ( i = 0; i < lcn_list_size( file_list ); i++ )
        {
            char *file_name = lcn_list_get( file_list, i );

            if( lcn_string_starts_with(file_name,pattern)  && isdigit( file_name[pattern_len] ) )
            {
                *flag = LCN_TRUE;
            }
        }
    }
    while(0);

    if ( NULL != child_pool )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}


/**
 * Lucene 4.0
 */

void
lcn_segment_infos_changed( lcn_segment_infos_t *segment_infos )
{
    segment_infos->version++;
}

void
lcn_segment_infos_clear( lcn_segment_infos_t *segment_infos )
{
    lcn_list_clear( segment_infos->segments );
}


static apr_status_t
generation_from_segments_file_name(char *file_name, apr_int64_t *gen)
{
    if ( 0 == strcmp( file_name, LCN_INDEX_FILE_NAMES_SEGMENTS ))
    {
        *gen = 0;
        return APR_SUCCESS;
    }

    if ( 0 == strncmp( file_name, LCN_INDEX_FILE_NAMES_SEGMENTS, strlen( LCN_INDEX_FILE_NAMES_SEGMENTS )) &&
         0 != strcmp( file_name, LCN_INDEX_FILE_NAMES_SEGMENTS_GEN ))
    {
        char *end = NULL;
        apr_int64_t generation = apr_strtoi64( file_name + 1 + strlen(LCN_INDEX_FILE_NAMES_SEGMENTS),  &end,
                                               LCN_INDEX_FILE_NAMES_MAX_RADIX );

        if ( end == (file_name + strlen(file_name)))
        {
            *gen = generation;
            return APR_SUCCESS;
        }
    }

    return LCN_ERR_SEGMENT_INFOS_INVALID_SEGMENTS_FILE_NAME;
}

static apr_status_t
get_last_commit_generation( lcn_list_t *files, apr_int64_t *gen )
{
    apr_status_t s = APR_SUCCESS;
    apr_int64_t max = -1;
    int i;

    do
    {
        if ( NULL == files || lcn_list_size( files ) == 0 )
        {
            *gen = -1;
            break;
        }

        for( i = 0; i < lcn_list_size( files ); i++ )
        {
            char *file_name = (char*) lcn_list_get( files, i );

            if ( 0 == strncmp( file_name, LCN_INDEX_FILE_NAMES_SEGMENTS, strlen( LCN_INDEX_FILE_NAMES_SEGMENTS )) &&
                 0 != strcmp( file_name, LCN_INDEX_FILE_NAMES_SEGMENTS_GEN ))
            {
                apr_int64_t generation;

                LCNCE( generation_from_segments_file_name(file_name, &generation));

                if (generation > max)
                {
                    max = generation;
                }
            }
        }

        *gen = max;
    }
    while(0);

    return s;
}

/**
 * List the directory and use the highest
 * segments_N file.  This method works well as long
 * as there is no stale caching on the directory
 * contents (NOTE: NFS clients often have such stale caching):
 */
static apr_status_t
find_segments_file( lcn_directory_t *directory,
                    char **segments_file,
                    apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_bool_t exists;
        lcn_list_t *file_list;
        apr_int64_t gen_a, gen_b, gen;
        lcn_index_input_t *gen_input = NULL;
        apr_status_t stat;

        /* first check for old format (still 2.4) */

        LCNCE( lcn_directory_file_exists ( directory, LCN_INDEX_FILE_NAMES_SEGMENTS, &exists ));

        if ( LCN_TRUE == exists )
        {
            *segments_file = apr_pstrdup( pool, LCN_INDEX_FILE_NAMES_SEGMENTS );
            break;
        }

        /* now assume we are Lucene 4.0 */

        LCNCE( lcn_directory_list( directory, &file_list, pool ) );
        LCNCE( get_last_commit_generation( file_list, &gen_a ));

        /**
         * Also open segments.gen and read its
         * contents.  Then we take the larger of the two
         * gens.  This way, if either approach is hitting
         * a stale cache (NFS) we have a better chance of
         * getting the right generation.
         */
        gen_b = -1;

        stat = lcn_directory_open_input( directory,
                                         &gen_input,
                                         LCN_INDEX_FILE_NAMES_SEGMENTS,
                                         LCN_IO_CONTEXT_READONCE,
                                         pool );

        if ( stat != LCN_ERR_NOT_REGULAR_FILE &&
             stat != LCN_ERR_RAM_FILE_NOT_FOUND )
        {
            LCNCE( stat );
        }

        if ( gen_input != NULL )
        {
            flog( stderr, "Implement TODO segment_infos:427\n" );
            return APR_ENOMEM;
#if 0
            try {
              int version = genInput.readInt();
              if (version == FORMAT_SEGMENTS_GEN_CURRENT) {
                long gen0 = genInput.readLong();
                long gen1 = genInput.readLong();
                if (infoStream != null) {
                  message("fallback check: " + gen0 + "; " + gen1);
                }
                if (gen0 == gen1) {
                  // The file is consistent.
                  genB = gen0;
                }
              } else {
                throw new IndexFormatTooNewException(genInput, version, FORMAT_SEGMENTS_GEN_CURRENT, FORMAT_SEGMENTS_GEN_CURRENT);
              }
            } catch (IOException err2) {
              // rethrow any format exception
              if (err2 instanceof CorruptIndexException) throw err2;
            } finally {
              genInput.close();
            }
#endif
        }

        gen = ( gen_a > gen_b ? gen_a : gen_b );

        LCNASSERT( gen != -1, LCN_ERR_INDEX_NOT_FOUND );

        /* more simplification's going on, fix it eventually */

        *segments_file = lcn_index_file_names_file_name_from_generation( LCN_INDEX_FILE_NAMES_SEGMENTS, "", gen, pool );

        flog( stderr, "segments_file:  %s\n", *segments_file );

        *segments_file = NULL;
    }
    while(0);

    return APR_SUCCESS;
}


apr_status_t
lcn_segment_infos_read_directory( lcn_segment_infos_t *segment_infos,
                                  lcn_directory_t *dir )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;
    lcn_index_input_t *is = NULL;

    do
    {
        int format, size, i;
        char *segments_file = NULL;

        LCNCE( apr_pool_create( &cp, segment_infos->pool ));

        // Clear any previous segments:
        lcn_segment_infos_clear( segment_infos );
        segment_infos->last_generation = segment_infos->generation;

        /**
         * WARNING: we simplify a lot here. Should be completed.
         */
        LCNCE( find_segments_file( dir, &segments_file, cp ));
        LCNASSERT( NULL != segments_file, LCN_ERR_NOT_REGULAR_FILE );

        if ( 0 == strcmp( LCN_INDEX_FILE_NAMES_SEGMENTS, segments_file ))
        {
            /* execute old code here (to delete as soon as possible)  */

            if (( s = lcn_directory_open_input( dir, &is, LCN_INDEX_FILE_NAMES_SEGMENTS, LCN_IO_CONTEXT_READONCE, cp )))
            {
                break;
            }

            LCNCE( lcn_index_input_read_int( is, &format ) );

            if ( format < 0 )  /* file contains explicit format info */
            {
                int counter;
                LCNASSERT( format >= LCN_SEGMENT_INFOS_FORMAT, LCN_ERR_SEGMENT_INFOS_UNKNOWN_FILE_FORMAT );
                LCNCE( lcn_index_input_read_ulong( is, &(segment_infos->version) ));
                LCNCE( lcn_index_input_read_int( is, &counter ) );
                segment_infos->counter = counter;
                segment_infos->format = format;
            }

            LCNCE( lcn_index_input_read_int( is, (int*)&size ));

            for( i = 0; i < size; i++ )
            {
                char *name;
                int doc_count;
                unsigned int len;

                LCNCE( lcn_index_input_read_string( is, &name, &len, lcn_list_pool( segment_infos->list )));
                LCNCE( lcn_index_input_read_int( is, (int*)&doc_count ) );
                LCNCE( lcn_segment_infos_add_info( segment_infos, dir, name, doc_count ));
            }

            dir->segments_format = segment_infos->format;
            dir->segments_format_is_init = LCN_TRUE;
        }
        else
        {
        /* Lucene 4.0 */
        }
    }
    while(0);

    if ( NULL != is )
    {
        apr_status_t stat = lcn_index_input_close( is );
        s = ( s ? s : stat );
    }

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_segment_infos_files( lcn_segment_infos_t *segment_infos,
                         lcn_directory_t *dir,
                         lcn_bool_t include_segments_file,
                         apr_pool_t *pool,
                         lcn_list_t **segment_file_names )
{
    apr_status_t s;
    apr_pool_t *cp;

    do
    {
        int i;
        char* hash_default_str = "";

        apr_pool_create( &cp, pool );

        LCNCE( lcn_list_create( segment_file_names, 0, pool ) );

        if ( include_segments_file )
        {

            apr_hash_t* files = apr_hash_make( cp );
            apr_hash_index_t* hi;

            char *segment_file_name = lcn_index_file_names_file_name_from_generation( LCN_INDEX_FILE_NAMES_SEGMENTS,
                                                                                      "",
                                                                                      segment_infos->last_generation,
                                                                                      pool );

            if ( NULL != segment_file_name )
            {
                /*
                 * TODO: if last_generation == -1 we might get null here it seems wrong to
                 * and null to the files set
                 */
                apr_hash_set( files, segment_file_name, APR_HASH_KEY_STRING, hash_default_str );
            }

            for(i = 0; i <= apr_hash_count( files ); i++ )
            {
                lcn_segment_info_per_commit_t* info = (lcn_segment_info_per_commit_t*) lcn_list_get( segment_infos->segments, i );

                if( info->segment_info->directory == dir )
                {
                    apr_hash_t* segment_info_file_names;
                    LCNCE( lcn_segment_info_files( info->segment_info, cp, &segment_info_file_names ) );
                    files = apr_hash_overlay( cp, files, segment_info_file_names );
                }
            }
            LCNCE( s );

            for( hi = apr_hash_first( cp, files ); hi; hi = apr_hash_next( hi )  )
            {
                const void* hash_key;
                apr_hash_this( hi, &hash_key, NULL, NULL );
                LCNCE( lcn_list_add( *segment_file_names, (char*) hash_key ) );
            }
            LCNCE( s );
        }
    }
    while(0);

    if( cp != NULL )
    {
        apr_pool_destroy( cp );
    }

    return s;

}

/** Call this to start a commit.  This writes the new
 *  segments file, but writes an invalid checksum at the
 *  end, so that it is not visible to readers.  Once this
 *  is called you must call {@link #finishCommit} to complete
 *  the commit or {@link #rollbackCommit} to abort it.
 *  <p>
 *  Note: {@link #changed()} should be called prior to this
 *  method if changes have been made to this {@link SegmentInfos} instance
 *  </p>
 **/
apr_status_t
lcn_segment_infos_prepare_commit( lcn_segment_infos_t *segment_infos,
                                  lcn_directory_t *dir )
{
    if ( segment_infos->pending_seqn_output != NULL )
    {
        return LCN_ERR_PREPARE_COMMIT_WAS_NOT_CALLED;
    }

    return lcn_segment_infos_write( segment_infos,
                                    dir );
}

apr_status_t
lcn_segment_infos_clone( lcn_segment_infos_t **clone,
                         lcn_segment_infos_t *segment_infos,
                         apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_list_t *list;

    *clone = lcn_object_create( lcn_segment_infos_t, pool );
    *clone = segment_infos;

    do
    {
        unsigned int i, size = lcn_list_size( segment_infos->segments );
        (*clone)->pool = pool;

        LCNCE( lcn_list_create( &list, size, pool ) );
        // deep clone, first recreate all collections:
        (*clone)->segments = list;

        for( i = 0; i < size; i++ )
        {
#if 0
            TODO: implement

            assert info.info.getCodec() != null;
#endif
            lcn_segment_info_per_commit_t *sipc = (lcn_segment_info_per_commit_t*) lcn_list_get( segment_infos->segments, i );
            LCNCE( lcn_list_add( list, lcn_segment_info_per_commit_clone( sipc, pool ) ) );
        }
    }
    while(0);

    return s;
}

void
lcn_segment_infos_update_generation( lcn_segment_infos_t *clone,
                                     lcn_segment_infos_t *segment_infos )
{
    segment_infos->last_generation = clone->last_generation;
    segment_infos->generation = clone->generation;
}

/**
 * A utility for writing the {@link IndexFileNames#SEGMENTS_GEN} file to a
 * {@link Directory}.
 *
 * <p>
 * <b>NOTE:</b> this is an internal utility which is kept public so that it's
 * accessible by code from other packages. You should avoid calling this
 * method unless you're absolutely sure what you're doing!
 *
 * @lucene.internal
 */
static void
lcn_segment_infos_write_segments_gen( lcn_directory_t *dir,
                                      unsigned int generation )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t* cp = NULL;
    lcn_index_output_t* gen_output = NULL;

    do
    {
        LCNCE( apr_pool_create( &cp, dir->pool ) );
        LCNCE( lcn_directory_create_output( dir,
                                            &gen_output,
                                            LCN_INDEX_FILE_NAMES_SEGMENTS_GEN,
                                            cp) );

        LCNCE( lcn_index_output_write_int( gen_output, LCN_INDEX_FILE_NAMES_FORMAT_SEGMENTS_GEN_CURRENT ) );
        LCNCE( lcn_index_output_write_long( gen_output, generation ) );
        LCNCE( lcn_index_output_write_long( gen_output, generation ) );
    }
    while( 0 );

    if ( NULL != gen_output )
    {
        s = lcn_index_output_close ( gen_output );
    }

    // It's OK if we fail to write this file since it's
    // used only as one of the retry fallbacks.
    if ( s )
    {
        // Ignore; this file is only used in a retry
        // fallback on init.
        lcn_directory_delete_file( dir, LCN_INDEX_FILE_NAMES_SEGMENTS_GEN );
    }

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }
}

apr_status_t
lcn_segment_infos_finish_commit( lcn_segment_infos_t *pending_commit,
                                 lcn_directory_t *directory )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *child_pool;

    if ( NULL == pending_commit->pending_seqn_output )
    {
        return LCN_ERR_PREPARE_COMMIT_WAS_NOT_CALLED;
    }

    do
    {
        //char *file_name;

        LCNCE( apr_pool_create( &child_pool, pending_commit->pool ) );
        LCNCE( lcn_checksum_index_output_finish_commit( pending_commit->pending_seqn_output ) );

#if 0
    TODO: Implement (rollbackCommit)

     if (!success) {
        // Closes pendingSegnOutput & deletes partial segments_N:
        rollbackCommit(dir);
      } else {
        success = false;
        try {
          pendingSegnOutput.close();
          success = true;
        } finally {
          if (!success) {
            // Closes pendingSegnOutput & deletes partial segments_N:
            rollbackCommit(dir);
          } else {
            pendingSegnOutput = null;
          }
        }
      }
#endif

        LCNCE( lcn_index_output_close( pending_commit->pending_seqn_output ) );

        pending_commit->pending_seqn_output = NULL;

#if 0
        //TODO: Hier muss noch geschaut werden, ob die Funktion überhaupt benötigt wird.

        // NOTE: if we crash here, we have left a segments_N
        // file in the directory in a possibly corrupt state (if
        // some bytes made it to stable storage and others
        // didn't).  But, the segments_N file includes checksum
        // at the end, which should catch this case.  So when a
        // reader tries to read it, it will throw a
        // CorruptIndexException, which should cause the retry
        // logic in SegmentInfos to kick in and load the last
        // good (previous) segments_N-1 file.

        file_name = lcn_index_file_names_file_name_from_generation( LCN_INDEX_FILE_NAMES_SEGMENTS, "", pending_commit->generation, child_pool );
#endif

#if 0
        TODO: Attention simplification ( just create new segment file and do not sync existing )

        final String fileName = IndexFileNames.fileNameFromGeneration(IndexFileNames.SEGMENTS, "", generation);

        success = false;
        try {
          dir.sync(Collections.singleton(fileName));
          success = true;
        } finally {
          if (!success) {
            try {
              dir.deleteFile(fileName);
            } catch (Throwable t) {
              // Suppress so we keep throwing the original exception
            }
          }
        }
#endif
        pending_commit->last_generation = pending_commit->generation;
        lcn_segment_infos_write_segments_gen( directory, pending_commit->generation );
    }
    while(0);

    if ( child_pool != NULL )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}
