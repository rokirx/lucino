#include "lucene.h"
#include "lcn_util.h"
#include "lcn_term_enum.h"
#include "lcn_search.h"
#include "lcn_segment_merge_info.h"

#include "fields_writer.h"
#include "segment_merger.h"
#include "document.h"
#include "term.h"
#include "segment_infos.h"
#include "term_infos_reader.h"
#include "index_reader.h"
#include "fields_reader.h"
#include "compound_file_writer.h"
#include "compound_file_util.h"


/*
#define CMP_EXT_COUNT (7)
char* COMPOUND_EXTENSIONS[CMP_EXT_COUNT] = { "fnm", "frq", "prx", "fdx", "fdt", "tii", "tis" };
*/
static char *
lcn_weighted_extension( int i, apr_pool_t *pool )
{
    return apr_pstrcat( pool, ".f", apr_itoa( pool, i ), NULL );
}

apr_status_t
lcn_segment_merger_close_readers( lcn_segment_merger_t *segment_merger )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i;

        for ( i = 0; i < lcn_list_size( segment_merger->readers ); i++ )
        {
            lcn_index_reader_t *reader;
            apr_status_t stat;

            reader = lcn_list_get( segment_merger->readers, i );
            stat = lcn_index_reader_close( reader );
            s = ( s ? s : stat );
        }
    }
    while(0);

    return s;
}

/**
 * Adds a reader of a segment to the merger. This segment
 * can be then merged by the merger.
 */
apr_status_t
lcn_segment_merger_add_reader ( lcn_segment_merger_t *segment_merger,
                                lcn_index_reader_t *index_reader )
{
    return lcn_list_add( segment_merger->readers, index_reader );
}

static apr_status_t
lcn_segment_merger_merge_fields( lcn_segment_merger_t *segment_merger,
                                 unsigned int* merged_doc_count )
{
    apr_status_t s;
    unsigned int i;
    apr_pool_t *pool = NULL;
    lcn_fields_writer_t *fields_writer = NULL;

    LCNCR( apr_pool_create( &pool, segment_merger->pool ) );

    do
    {
        lcn_list_t *list;
        char *fnm_file;

        *merged_doc_count = 0;

        LCNCE( lcn_list_create( &list, 10, pool ) );
        LCNCE( lcn_field_infos_create( &segment_merger->field_infos, segment_merger->pool ));

        for ( i = 0; i < lcn_list_size( segment_merger->readers ); i++ )
        {
            lcn_index_reader_t *reader = lcn_list_get( segment_merger->readers, i );

            /* get IndexReader.FieldOption.TERMVECTOR_WITH_POSITION_OFFSET fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV,
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ));

            /* get IndexReader.FieldOption.TERMVECTOR_WITH_POSITION fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   LCN_FIELD_STORE_POSITION_WITH_TV,
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ));

            /* get IndexReader.FieldOption.TERMVECTOR_WITH_OFFSET fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   LCN_FIELD_STORE_OFFSET_WITH_TV,
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ));

            /* get IndexReader.FieldOption.TERMVECTOR fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   LCN_FIELD_STORE_TERM_VECTOR,
                   LCN_FIELD_STORE_TERM_VECTOR | LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ));

            /* get IndexReader.FieldOption.INDEXED fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   LCN_FIELD_INDEXED,
                   LCN_FIELD_INDEXED | LCN_FIELD_STORE_TERM_VECTOR |
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ));

            /* get IndexReader.FieldOption.UNINDEXED fields */

            LCNCE( lcn_index_reader_get_field_infos( reader, list,
                   0,
                   LCN_FIELD_INDEXED | LCN_FIELD_STORE_TERM_VECTOR |
                   LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ) );
        }

        if ( s )
        {
            break;
        }

        for ( i = 0; i < lcn_list_size( list ); i++ )
        {
            lcn_field_info_t *fi;

            fi = lcn_list_get( list, i );

            LCNCE( lcn_field_infos_add_field_info( segment_merger->field_infos,
                                                   lcn_field_info_name( fi ),
                                                   fi->bits ));
        }

        if ( s )
        {
            break;
        }

        LCNPV( fnm_file = apr_pstrcat( pool, segment_merger->segment,
                                       ".fnm",
                                       NULL ),
               APR_ENOMEM );

        LCNCE( lcn_field_infos_write( segment_merger->field_infos,
                                      segment_merger->directory,
                                      fnm_file ));

        LCNCE( lcn_fields_writer_create( &fields_writer,
                                         segment_merger->directory,
                                         segment_merger->segment,
                                         segment_merger->field_infos,
                                         pool ));

        for ( i = 0; i < lcn_list_size( segment_merger->readers ); i++ )
        {
            unsigned int max_doc;
            lcn_index_reader_t *reader;
            unsigned int j;
            apr_pool_t *doc_pool;

            LCNCE( apr_pool_create( &doc_pool, pool ) );

            reader = lcn_list_get( segment_merger->readers, i );

            max_doc = lcn_index_reader_max_doc( reader );

            for ( j = 0; j < max_doc; j++ )
            {
                if ( ! lcn_index_reader_is_deleted( reader, j ) )
                {
                    lcn_document_t *doc;

                    LCNCE( lcn_index_reader_document( reader, &doc, j, doc_pool ) );
                    LCNCE( lcn_fields_writer_add_document( fields_writer, doc ) );

                    (*merged_doc_count)++;

                    apr_pool_clear( doc_pool );
                }
            }

            if ( s )
            {
                break;
            }
        }
    }
    while(0);

    if ( NULL != fields_writer )
    {
        apr_status_t stat = lcn_fields_writer_close( fields_writer );
        s = s ? s : stat;
    }

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}

static apr_status_t
lcn_segment_merger_reset_skip( lcn_segment_merger_t *segment_merger )
{
    apr_status_t s;
    LCNCR( lcn_ram_ostream_reset( segment_merger->skip_buffer ) );
    segment_merger->last_skip_doc = 0;
    segment_merger->last_skip_freq_pointer = lcn_ostream_get_file_pointer( segment_merger->freq_output );
    segment_merger->last_skip_prox_pointer = lcn_ostream_get_file_pointer( segment_merger->prox_output );
    return s;
}

static apr_status_t
lcn_segment_merger_buffer_skip( lcn_segment_merger_t *segment_merger, unsigned int doc )
{
    apr_status_t s;

    apr_off_t freq_pointer = lcn_ostream_get_file_pointer( segment_merger->freq_output );
    apr_off_t prox_pointer = lcn_ostream_get_file_pointer( segment_merger->prox_output );

    LCNCR( lcn_ostream_write_vint( segment_merger->skip_buffer, doc - segment_merger->last_skip_doc ));
    LCNCR( lcn_ostream_write_vint( segment_merger->skip_buffer,
                                   (unsigned int) (freq_pointer-segment_merger->last_skip_freq_pointer)));
    LCNCR( lcn_ostream_write_vint( segment_merger->skip_buffer,
                                   (unsigned int) (prox_pointer-segment_merger->last_skip_prox_pointer)));

    segment_merger->last_skip_doc = doc;
    segment_merger->last_skip_freq_pointer = freq_pointer;
    segment_merger->last_skip_prox_pointer = prox_pointer;

    return s;
}

static apr_status_t
lcn_segment_merger_append_postings( lcn_segment_merger_t *segment_merger,
                                    lcn_segment_merge_info_t **smis,
                                    int n,
                                    unsigned int *docs_count )
{
    apr_status_t s;

    do
    {
        unsigned int last_doc = 0;
        unsigned int df = 0; /* number of docs w/ term */
        unsigned int i;

        LCNCE( lcn_segment_merger_reset_skip( segment_merger ) );

        for( i = 0; i < n; i++ )
        {
            apr_status_t status_next;
            unsigned int base;
            lcn_int_array_t *doc_map;
            lcn_segment_merge_info_t *smi = smis[i];
            lcn_term_docs_t *postings;

            LCNCE( lcn_segment_merge_info_get_positions( smi, &postings ));
            base = smi->base;

            LCNCE( lcn_segment_merge_info_get_doc_map( smi, &doc_map ) );
            LCNCE( lcn_term_docs_seek_term_enum( postings, smi->term_enum ));

            while( APR_SUCCESS == ( status_next = lcn_term_docs_next( postings ) ))
            {
                unsigned int j;
                unsigned int last_position;
                unsigned int freq;
                unsigned int doc_code;
                unsigned int doc = lcn_term_docs_doc( postings );

                if ( NULL != doc_map )
                {
                    doc = doc_map->arr[ doc ];   /* map around deletions */
                }

                doc += base;                     /* convert to merged space */

                LCNASSERT( doc >= last_doc, LCN_ERR_DOCS_OUT_OF_ORDER );

                df++;

                if ( 0 == ( df % segment_merger->skip_interval ) )
                {
                    LCNCE( lcn_segment_merger_buffer_skip( segment_merger, last_doc ));
                }

                /* use low bit to flag freq=1 */

                doc_code = (doc - last_doc) << 1;
                last_doc = doc;

                freq = lcn_term_docs_freq( postings );

                if ( 1 == freq )
                {
                    /* write doc & freq=1 */
                    LCNCE( lcn_ostream_write_vint( segment_merger->freq_output, doc_code | 1 ));
                }
                else
                {
                    LCNCE( lcn_ostream_write_vint( segment_merger->freq_output, doc_code ) ); /* write doc         */
                    LCNCE( lcn_ostream_write_vint( segment_merger->freq_output, freq     ) ); /* write freq in doc */
                }

                /* write position deltas */

                last_position = 0;

                for ( j = 0; j < freq; j++ )
                {
                    apr_ssize_t position;
                    LCNCE( lcn_term_positions_next_position( postings, &position ));
                    LCNCE( lcn_ostream_write_vint( segment_merger->prox_output, position - last_position ));
                    last_position = position;
                }
            }

            if ( ! APR_SUCCESS == status_next &&
                 ! LCN_ERR_ITERATOR_NO_NEXT == status_next )
            {
                s = ( APR_SUCCESS == s ? status_next : s );
            }
        }

        *docs_count = df;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_merger_write_skip( lcn_segment_merger_t *segment_merger,
                               apr_off_t *skip_pointer)
{
    apr_status_t s;
    *skip_pointer = lcn_ostream_get_file_pointer( segment_merger->freq_output );
    LCNCR( lcn_ram_ostream_write_to( segment_merger->skip_buffer, segment_merger->freq_output ));
    return s;
}

static apr_status_t
lcn_segment_merger_merge_term_info( lcn_segment_merger_t *segment_merger,
                                    lcn_segment_merge_info_t **smis,
                                    unsigned int n )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        apr_off_t freq_pointer = lcn_ostream_get_file_pointer( segment_merger->freq_output );
        apr_off_t prox_pointer = lcn_ostream_get_file_pointer( segment_merger->prox_output );
        unsigned int df;
        apr_off_t skip_pointer;

        LCNCE( lcn_segment_merger_append_postings( segment_merger, smis, n, &df ));
        LCNCE( lcn_segment_merger_write_skip( segment_merger, &skip_pointer ));

        if ( df > 0 )
        {
            /* add an entry to the dictionary with pointers to prox and freq files */

            unsigned int field_number;
            const lcn_term_t *term = smis[0]->term;

            segment_merger->term_info->doc_freq = df;
            segment_merger->term_info->freq_pointer = freq_pointer;
            segment_merger->term_info->prox_pointer = prox_pointer;
            segment_merger->term_info->skip_offset = (apr_off_t) ( skip_pointer - freq_pointer );

            LCNCE( lcn_field_infos_field_number( segment_merger->field_infos,
                                                 &field_number,
                                                 lcn_term_field( term )));

            LCNCM( lcn_term_infos_writer_add_term ( segment_merger->ti_writer,
                                                    term,
                                                    segment_merger->term_info,
                                                    field_number ),
                   lcn_term_text( term ) );
        }
    }
    while(0);

    return s;
}


static apr_status_t
lcn_segment_merger_merge_term_infos ( lcn_segment_merger_t *segment_merger,
                                      apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int base = 0;
        unsigned int i;
        lcn_segment_merge_info_t **match;

        for( i = 0; i < lcn_list_size( segment_merger->readers ); i++ )
        {
            lcn_segment_merge_info_t *smi;
            lcn_term_enum_t *term_enum;
            lcn_index_reader_t *reader = lcn_list_get( segment_merger->readers, i );

            LCNCE( lcn_index_reader_terms( reader, &term_enum, pool ));
            LCNCE( lcn_segment_merge_info_create( &smi, base, term_enum, reader, pool ));

            base += lcn_index_reader_num_docs( reader );

            if ( APR_SUCCESS == ( s = lcn_segment_merge_info_next( smi )))
            {
                lcn_priority_queue_put( segment_merger->queue, smi ); /* initialize queue */
            }
            else
            {
                LCNCE( lcn_segment_merge_info_close( smi ));
                s = ( LCN_ERR_ITERATOR_NO_NEXT == s ? APR_SUCCESS : s );
            }
        }

        if( s )
        {
            break;
        }

        LCNPV( match = (lcn_segment_merge_info_t**) apr_pcalloc(pool, sizeof(lcn_segment_merge_info_t*) *
                                                                lcn_list_size( segment_merger->readers )),
               APR_ENOMEM );

        while( lcn_priority_queue_size( segment_merger->queue ) > 0 )
        {
            unsigned int match_size = 0;    /* pop matching terms */
            const lcn_term_t *term;
            lcn_segment_merge_info_t *top;

            match[ match_size++ ] = (lcn_segment_merge_info_t*) lcn_priority_queue_pop( segment_merger->queue );
            term = match[0]->term;

            top = (lcn_segment_merge_info_t*) lcn_priority_queue_top( segment_merger->queue );

            while( NULL != top && lcn_term_compare( term, top->term ) == 0 )
            {
                match[ match_size++ ] = (lcn_segment_merge_info_t*) lcn_priority_queue_pop( segment_merger->queue );
                top = (lcn_segment_merge_info_t*) lcn_priority_queue_top( segment_merger->queue );
            }

            /* add new TermInfo */

            LCNCE( lcn_segment_merger_merge_term_info( segment_merger, match, match_size ));

            while( match_size > 0 )
            {
                lcn_segment_merge_info_t *smi = match[--match_size];

                if ( APR_SUCCESS == ( s = lcn_segment_merge_info_next( smi ) ))
                {
                    lcn_priority_queue_put( segment_merger->queue, smi );  /* restore queue */
                }
                else
                {
                    LCNCE( lcn_segment_merge_info_close( smi ));           /* done with a segment */
                    s = ( LCN_ERR_ITERATOR_NO_NEXT == s ? APR_SUCCESS : s );
                }
            }

            if ( s )
            {
                break;
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_merger_merge_terms( lcn_segment_merger_t *segment_merger )
{
    apr_status_t s;
    lcn_field_info_t *fi = segment_merger->field_infos->first_info;
    apr_pool_t *pool = NULL;

    /* check first whether we have indexed fields */

    while ( fi != NULL )
    {
        if ( lcn_field_info_is_indexed( fi ) )
        {
            break;
        }

        fi = fi->next;
    }

    if ( fi == NULL )
    {
        return APR_SUCCESS;
    }

    /* ok, there some indexed fields there, so we must work */

    do
    {
        LCNCE( apr_pool_create( &pool, segment_merger->pool ) );

        LCNCE( lcn_directory_create_segment_file ( segment_merger->directory,
                                                   &(segment_merger->freq_output),
                                                   segment_merger->segment,
                                                   ".frq",
                                                   pool ) );

        LCNCE( lcn_directory_create_segment_file ( segment_merger->directory,
                                                   &(segment_merger->prox_output),
                                                   segment_merger->segment,
                                                   ".prx",
                                                   pool ) );

        LCNCE( lcn_term_infos_writer_create( &(segment_merger->ti_writer),
                                             segment_merger->directory,
                                             segment_merger->segment,
                                             segment_merger->term_index_interval,
                                             pool ));

        segment_merger->skip_interval = segment_merger->ti_writer->skip_interval;

        LCNCE( lcn_segment_merge_queue_create( &(segment_merger->queue),
                                               lcn_list_size( segment_merger->readers ),
                                               pool ));

        LCNCE( lcn_segment_merger_merge_term_infos( segment_merger, pool ) );
    }
    while(0);

    /* finally */
    {
        apr_status_t stat = APR_SUCCESS;

        if ( NULL != segment_merger->freq_output )
        {
            stat = lcn_ostream_close( segment_merger->freq_output );
            segment_merger->freq_output = NULL;
        }

        s = s ? s : stat;

        if ( NULL != segment_merger->prox_output )
        {
            stat = lcn_ostream_close( segment_merger->prox_output );
            segment_merger->prox_output = NULL;
        }

        s = s ? s : stat;

        if ( NULL != segment_merger->ti_writer )
        {
            stat = lcn_term_infos_writer_close( segment_merger->ti_writer );
            segment_merger->ti_writer = NULL;
        }

        s = s ? s : stat;

        if ( NULL != segment_merger->queue )
        {
            stat = lcn_segment_merge_queue_close( segment_merger->queue );
            segment_merger->queue = NULL;
        }

        s = s ? s : stat;

        if ( NULL != pool )
        {
            apr_pool_destroy( pool );
        }
    }

    return s;
}

static apr_status_t
lcn_fake_norms( char *buf,
                unsigned int buf_size,
                apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t *p = NULL;

    do
    {
        lcn_similarity_t* similarity;
        unsigned int i;
        lcn_byte_t b;

        LCNCE( apr_pool_create( &p, pool ));
        LCNCE( lcn_default_similarity_create( &similarity, p ));

        b = lcn_similarity_encode_norm( similarity, (float) 1.0 );

        for( i = 0; i < buf_size; i++ )
        {
            buf[ i ] = b;
        }
    }
    while(0);

    if ( NULL != p )
    {
        apr_pool_destroy( p );
    }

    return s;
}

static apr_status_t
lcn_segment_merger_merge_norms( lcn_segment_merger_t *segment_merger )
{
    apr_status_t s;
    apr_pool_t *pool;

    LCNCR( apr_pool_create( &pool, segment_merger->pool ));

    do
    {
        unsigned int fi_size = lcn_field_infos_size( segment_merger->field_infos );
        unsigned int i;

        for( i = 0; i < fi_size; i++ )
        {
            lcn_field_info_t *field_info;

            LCNCE( lcn_field_infos_by_number( segment_merger->field_infos,
                                              &field_info,
                                              i ));

            if ( lcn_field_info_is_indexed( field_info ) &&
                 ! lcn_field_info_omit_norms( field_info ) )
            {
                lcn_ostream_t *output = NULL;
                unsigned int j;

                LCNCE( lcn_directory_create_segment_file ( segment_merger->directory,
                                                           &output,
                                                           segment_merger->segment,
                                                           lcn_weighted_extension( i, pool ),
                                                           pool ));

                for ( j = 0; j < lcn_list_size( segment_merger->readers ); j++ )
                {
                    lcn_byte_array_t *input;
                    lcn_index_reader_t *reader = lcn_list_get( segment_merger->readers, j );
                    unsigned int max_doc = lcn_index_reader_max_doc( reader );
                    unsigned int k;

                    LCNCE( lcn_byte_array_create( &input,
                                                  max_doc,
                                                  pool ));

                    /* while merging segments there may be segments containing   */
                    /* documents without this field, therefore no norms. In this */
                    /* case just fake norms                                      */

                    if ( APR_SUCCESS != lcn_index_reader_read_norms_to_array( reader,
                                                                              input,
                                                                              0,
                                                                              lcn_field_info_name( field_info )))
                    {
                        LCNCE( lcn_fake_norms( input->arr, max_doc , pool ));
                    }

                    for( k = 0; k < max_doc; k++ )
                    {
                        if ( ! lcn_index_reader_is_deleted( reader, k ))
                        {
                            LCNCE( lcn_ostream_write_byte( output, input->arr[ k ] ));
                        }
                    }
                }

                if ( NULL != output )
                {
                    apr_status_t stat = lcn_ostream_close( output );
                    s = ( s ? s : stat );
                }
            }

            if ( s )
            {
                break;
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

apr_status_t
lcn_segment_merger_merge( lcn_segment_merger_t *segment_merger,
                          unsigned int *merged_doc_count )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_segment_merger_merge_fields( segment_merger, merged_doc_count ));
        LCNCE( lcn_segment_merger_merge_terms( segment_merger ) );
        LCNCE( lcn_segment_merger_merge_norms( segment_merger ));

#if 0
    if (fieldInfos.hasVectors())
      mergeVectors();
#endif
        
    }
    while(0);

    return s;
}

apr_status_t
lcn_segment_merger_create( lcn_segment_merger_t **segment_merger,
                           lcn_index_writer_t *index_writer,
                           const char *merge_name,
                           apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_ram_file_t *ram_file;

        LCNPV( *segment_merger = apr_pcalloc( pool, sizeof(lcn_segment_merger_t) ),
               APR_ENOMEM );

        LCNPV( (*segment_merger)->segment = apr_pstrdup( pool, merge_name ), APR_ENOMEM );
        LCNCE( lcn_list_create( &((*segment_merger)->readers), 10, pool ) );
        LCNCE( lcn_ram_file_create ( &ram_file, pool ));
        LCNCE( lcn_ram_ostream_create( &((*segment_merger)->skip_buffer), ram_file, pool ));
        LCNCE( lcn_term_info_create( &((*segment_merger)->term_info), 0, 0, 0, 0, pool ));

        (*segment_merger)->directory = index_writer->directory;
        (*segment_merger)->term_index_interval = lcn_index_writer_get_term_index_interval( index_writer );
        (*segment_merger)->pool = pool;
    }
    while(0);

    return s;
}

#if 0
apr_status_t
lcn_segment_merger_create_compound_file( lcn_segment_merger_t *sm,
                                         char *cf_name,
                                         lcn_directory_t *dir,
                                         lcn_list_t **files,
                                         apr_pool_t *pool)
{
    apr_status_t s = APR_SUCCESS;
    int i = 0;
    lcn_compound_file_writer_t *cfw;
    lcn_compound_file_writer_create( &cfw, dir, cf_name, pool);
    
    
    do
    {
        unsigned int field_infos_size = lcn_field_infos_size(sm->field_infos);
        LCNCE( lcn_list_create( files, CP_EXT_COUNT + field_infos_size , pool ) );
        
        //Basic files
        for ( i = 0; i < CP_EXT_COUNT; i++)
        {
            LCNCE( lcn_list_add( *files, apr_pstrcat( pool, sm->segment, ".", COMPOUND_EXTENSIONS[i], NULL) ) );
        }
        LCNCE(s);
        
        // Field norm files
        lcn_field_info_t *field_info = sm->field_infos->first_info;
        int count = 0;
        while ( field_info != NULL )
        {
            if( lcn_field_info_is_indexed( field_info ) && !lcn_field_info_omit_norms( field_info )  )
            {
                lcn_list_add( *files, apr_pstrcat( pool, sm->segment, ".f", apr_itoa( pool, count ), NULL ) );
            }
            count++;
            field_info = field_info->next;
        }
        
        // Now merge all added files
        i = 0;
        for ( i = 0; i < lcn_list_size( *files ); i++ )
        {
            lcn_compound_file_writer_add_file( cfw, lcn_list_get( *files, i ) );
        }
        lcn_compound_file_writer_close( cfw );
        
    }
    while(0);
    
    return s;
}
#endif

/*
 final Vector createCompoundFile(String fileName)
          throws IOException {
    CompoundFileWriter cfsWriter =
            new CompoundFileWriter(directory, fileName);

    Vector files =
      new Vector(IndexFileNames.COMPOUND_EXTENSIONS.length + fieldInfos.size());    
    
    // Basic files
    for (int i = 0; i < IndexFileNames.COMPOUND_EXTENSIONS.length; i++) {
      files.add(segment + "." + IndexFileNames.COMPOUND_EXTENSIONS[i]);
    }

    // Field norm files
    for (int i = 0; i < fieldInfos.size(); i++) {
      FieldInfo fi = fieldInfos.fieldInfo(i);
      if (fi.isIndexed && !fi.omitNorms) {
        files.add(segment + ".f" + i);
      }
    }

    // Vector files
    if (fieldInfos.hasVectors()) {
      for (int i = 0; i < IndexFileNames.VECTOR_EXTENSIONS.length; i++) {
        files.add(segment + "." + IndexFileNames.VECTOR_EXTENSIONS[i]);
      }
    }

    // Now merge all added files
    Iterator it = files.iterator();
    while (it.hasNext()) {
      cfsWriter.addFile((String) it.next());
    }
    
    // Perform the merge
    cfsWriter.close();
   
    return files;
  }*/