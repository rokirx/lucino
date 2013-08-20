#include "lucene.h"
#include "lcn_index.h"
#include "lcn_store.h"
#include "document_writer.h"
#include "fields_writer.h"
#include "term.h"
#include "term_infos_writer.h"


typedef struct lcn_posting_t lcn_posting_t;

struct lcn_posting_t {
    lcn_term_t *term;
    unsigned int freq;

    unsigned int positions_size;
    unsigned int *positions;

    unsigned int offsets_size;
    lcn_term_vector_offset_info_t **offsets;
};

static apr_status_t
lcn_posting_create( lcn_posting_t **posting,
                    const char* field_name,
                    const char* term_text,
                    unsigned int position,
                    lcn_term_vector_offset_info_t *offset,
                    apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *posting = (lcn_posting_t*) apr_palloc( pool, sizeof(lcn_posting_t) ),
               APR_ENOMEM );

        LCNCE( lcn_term_create( &((*posting)->term),
                                field_name,
                                term_text,
                                LCN_TERM_TEXT_COPY,
                                pool ));
        (*posting)->freq = 1;

        (*posting)->positions_size = 1;
        LCNPV( (*posting)->positions = (unsigned int*) apr_palloc( pool, sizeof(unsigned int) ),
               APR_ENOMEM );
        (*posting)->positions[0] = position;

        if ( NULL != offset )
        {
            LCNPV( (*posting)->offsets = (lcn_term_vector_offset_info_t**)
                   apr_palloc( pool, sizeof(lcn_term_vector_offset_info_t*) ),
                   APR_ENOMEM );
            ((*posting)->offsets)[0] = offset;
        }
        else
        {
            (*posting)->offsets = NULL;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_document_writer_create( lcn_document_writer_t **document_writer,
                            lcn_directory_t *directory,
                            lcn_index_writer_t *index_writer,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *document_writer = (lcn_document_writer_t*) apr_pcalloc( pool, sizeof(lcn_document_writer_t)),
               APR_ENOMEM );

        LCNCE( apr_pool_create( &((*document_writer)->posting_table_pool), pool ) );
        LCNPV( (*document_writer)->posting_table = apr_hash_make((*document_writer)->posting_table_pool ),
               APR_ENOMEM );

        LCNCE( lcn_term_create( &((*document_writer)->term), "", "", LCN_TERM_NO_TEXT_COPY, pool ) );

        (*document_writer)->pool = pool;
        (*document_writer)->directory = directory;
        (*document_writer)->max_field_length = lcn_index_writer_get_max_field_length( index_writer );
        (*document_writer)->term_index_interval = lcn_index_writer_get_term_index_interval( index_writer );
        (*document_writer)->similarity = lcn_index_writer_get_similarity( index_writer );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_document_writer_add_position( lcn_document_writer_t *document_writer,
                                  const char *field,
                                  const char *text,
                                  unsigned int position,
                                  lcn_term_vector_offset_info_t *offset )
{
    apr_status_t s;

    do
    {
        lcn_posting_t *ti = (lcn_posting_t*) apr_hash_get( document_writer->field_postings,
                                                           text,
                                                           APR_HASH_KEY_STRING );
        lcn_term_set( document_writer->term, field, text );

        if ( NULL != ti ) /* word seen before */
        {
            unsigned int freq = ti->freq;

            if ( ti->positions_size == freq ) /* positions array is full   */
            {
                unsigned int *new_positions;
                unsigned int i;

                ti->positions_size *= 2;      /* double size               */
                LCNPV( new_positions = (unsigned int*) apr_palloc( document_writer->posting_table_pool,
                                                                 ti->positions_size * sizeof(unsigned int) ),
                       APR_ENOMEM );

                for( i = 0; i < freq; i++ )   /* copy old positions to new */
                {
                    new_positions[i] = ti->positions[i];
                }

                ti->positions = new_positions;
            }

            ti->positions[freq] = position;   /* add new position          */

            if ( NULL != offset )
            {
                if ( ti->offsets_size == freq )
                {
                    lcn_term_vector_offset_info_t **new_offsets;
                    unsigned int i;

                    ti->offsets_size *= 2;

                    LCNPV( new_offsets = (lcn_term_vector_offset_info_t**) apr_palloc( document_writer->posting_table_pool,
                                                   ti->offsets_size * sizeof(lcn_term_vector_offset_info_t*) ),
                           APR_ENOMEM );

                    for( i = 0; i < freq; i++ )
                    {
                        new_offsets[i] = ti->offsets[i];
                    }

                    ti->offsets = new_offsets;
                }

                ti->offsets[freq] = offset;
            }

            ti->freq = freq + 1;  /* update frequency */
        }
        else
        {
            lcn_posting_t *posting;

            LCNCE( lcn_posting_create( &posting,
                                       field,
                                       text,
                                       position,
                                       offset,
                                       document_writer->posting_table_pool ));

            apr_hash_set( document_writer->field_postings,
                          text,
                          APR_HASH_KEY_STRING,
                          posting );
        }
    }
    while(0);

    return APR_SUCCESS;
}

static apr_status_t
lcn_document_writer_activate_field_postings( lcn_document_writer_t *document_writer,
                                             const char *field_name )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        document_writer->field_postings = (apr_hash_t*) apr_hash_get( document_writer->posting_table,
                                                                      field_name,
                                                                      APR_HASH_KEY_STRING );

        if ( NULL == document_writer->field_postings )
        {
            LCNPV( document_writer->field_postings = apr_hash_make( document_writer->posting_table_pool ),
                   APR_ENOMEM );
            apr_hash_set( document_writer->posting_table,
                          field_name,
                          APR_HASH_KEY_STRING,
                          document_writer->field_postings );
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_document_writer_invert_document( lcn_document_writer_t *document_writer,
                                     lcn_document_t *doc )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_list_t *fields = lcn_document_get_fields( doc );
        unsigned int i;

        for( i = 0; i < lcn_list_size( fields ); i++ )
        {
            lcn_field_t *field;
            const char* field_name;
            unsigned int field_number;
            unsigned int length   = 0;
            unsigned int position = 0;
            unsigned int offset   = 0;

            LCNPV( field = lcn_list_get( fields, i ), LCN_ERR_INDEX_OUT_OF_RANGE );

            if ( lcn_field_is_indexed( field ))
            {
                field_name = lcn_field_name( field );
                LCNCE( lcn_document_writer_activate_field_postings( document_writer,
                                                                    field_name ) );

                LCNCE( lcn_field_infos_field_number( document_writer->field_infos,
                                                     &field_number,
                                                     field_name ) );
                length = document_writer->field_lengths[ field_number ];
                position = document_writer->field_positions[ field_number ];
                offset = document_writer->field_offsets[ field_number ];

                if ( ! lcn_field_is_tokenized( field ) )    /* untokenized field */
                {
                    if ( lcn_field_store_offset_with_term_vector( field ) )
                    {
                        fprintf(stderr, "TODO: store offset with term vector not implemented\n");
                        // addPosition(fieldName, stringValue, position++, new TermVectorOffsetInfo(offset, offset + stringValue.length()));
                    }
                    else
                    {
                        LCNCE( lcn_document_writer_add_position( document_writer,
                                                                 field_name,
                                                                 lcn_field_value( field ),
                                                                 position++,
                                                                 NULL ));
                    }

                    offset += strlen( lcn_field_value( field ) );
                    length++;
                }
                else
                {
                    lcn_analyzer_t *analyzer;
                    lcn_token_stream_t *token_stream;
                    lcn_token_t *t;

                    LCNCE( lcn_field_get_analyzer( field, &analyzer ) );

                    if ( length > 0 )
                    {
                        position += lcn_analyzer_get_position_increment_gap( analyzer );
                    }

                    /* tokenize field and add to posting table */

                    LCNCE( lcn_analyzer_token_stream( analyzer,
                                                      &token_stream,
                                                      lcn_field_value( field ),
                                                      document_writer->pool ) );

                    while( APR_SUCCESS == lcn_token_stream_next( token_stream, &t) )
                    {
                        char *term_text;
                        LCNCE( lcn_token_term_text( t, &term_text, document_writer->pool ) );
                        position += lcn_token_get_position_increment( t ) - 1;

                        if ( lcn_field_store_offset_with_term_vector( field ) )
                        {
                            fprintf(stderr, "ADD POSITION\n");
                            // addPosition(fieldName, t.termText(), position++, new TermVectorOffsetInfo(offset + t.startOffset(), offset + t.endOffset()));
                        }
                        else
                        {
                            char *term_text;

                            LCNCE( lcn_token_term_text( t, &term_text, document_writer->pool ));
                            LCNCE( lcn_document_writer_add_position( document_writer,
                                                                     field_name,
                                                                     term_text,
                                                                     position++,
                                                                     NULL ));
                        }

                        length++;
#if 0
                        if ( length > maxFieldLength) // TODO
                        {
                            if (infoStream != null)
                                infoStream.println("maxFieldLength " +maxFieldLength+ " reached, ignoring following tokens");
                            break;
                        }
#endif
                    }
#if 0
                    if(lastToken != null)  // TODO
                    {
                        offset += lastToken.endOffset() + 1;
                    }
#endif
                }

                document_writer->field_lengths[ field_number ]   = length;
                document_writer->field_positions[ field_number ] = position;
                document_writer->field_boosts[ field_number ]   *= lcn_field_get_boost( field );
                document_writer->field_offsets[ field_number ]   = offset;
            }
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_document_writer_init_posting_table( lcn_document_writer_t *document_writer,
                                        unsigned int field_infos_size,
                                        float boost )
{
    apr_status_t s;

    do
    {
        unsigned int i;

        apr_pool_clear( document_writer->posting_table_pool );

        LCNPV( document_writer->posting_table = apr_hash_make( document_writer->posting_table_pool ),
               APR_ENOMEM );

        LCNPV( document_writer->field_lengths = (unsigned int*) apr_pcalloc( document_writer->posting_table_pool,
                                                                           sizeof(unsigned int) * field_infos_size ),
               APR_ENOMEM );

        LCNPV( document_writer->field_positions = (unsigned int*) apr_pcalloc( document_writer->posting_table_pool,
                                                                             sizeof(unsigned int) * field_infos_size ),
               APR_ENOMEM );

        LCNPV( document_writer->field_offsets = (unsigned int*) apr_pcalloc( document_writer->posting_table_pool,
                                                                           sizeof(unsigned int) * field_infos_size ),
               APR_ENOMEM );

        LCNPV( document_writer->field_boosts = (float*) apr_palloc( document_writer->posting_table_pool,
                                                                    sizeof(float) * field_infos_size ),
               APR_ENOMEM );

        for( i = 0; i < field_infos_size; i++ )
        {
            (document_writer->field_boosts)[i] = boost;
        }

    }
    while(0);

    return s;
}

static int
lcn_document_writer_compare_postings( const void *p1, const void *p2 )
{
    return strcmp( lcn_term_text( (*((lcn_posting_t**)p1))->term ),
                   lcn_term_text( (*((lcn_posting_t**)p2))->term ) );
}

static int
lcn_document_writer_compare_postings_arrays( const void *p1, const void *p2 )
{
    return strcmp( lcn_term_field( ((lcn_posting_t*) ((*((lcn_ptr_array_t**)p1))->arr[0]))->term ),
                   lcn_term_field( ((lcn_posting_t*) ((*((lcn_ptr_array_t**)p2))->arr[0]))->term ) );
}

static apr_status_t
lcn_document_writer_sort_postings_table( lcn_document_writer_t *document_writer )
{
    apr_status_t s;

    do
    {
        unsigned int i = 0;
        unsigned int p_count = 0;
        apr_hash_index_t *hi;

        /* fill lcn_array_t document_writer->sorted_postings with */
        /* hash tables of type <term.text:lcn_postin_t> for       */
        /* corresponding fields.                                  */

        LCNCE( lcn_ptr_array_create( &(document_writer->sorted_postings),
                                     (unsigned int) apr_hash_count( document_writer->posting_table ),
                                     document_writer->posting_table_pool ));

        for( hi = apr_hash_first( document_writer->posting_table_pool,
                                  document_writer->posting_table );
             hi;
             hi = apr_hash_next(hi))
        {
            void *val;

            apr_hash_this( hi, NULL, NULL, &val );

            if ( 0 <  apr_hash_count(((apr_hash_t*) val))  )
            {
                document_writer->sorted_postings->arr[ p_count++ ] = val;
            }
        }

        /* fix the length of the sorted_postings array, as some fields may */
        /* have 0 terms and are not therefore to be sorted                 */

        document_writer->sorted_postings->length = p_count;

        /* sort each of the sorted_postings tables using qsort             */

        for( i = 0; i < document_writer->sorted_postings->length; i++ )
        {
            apr_hash_t *ph = (apr_hash_t*) document_writer->sorted_postings->arr[i];
            lcn_ptr_array_t *parr;
            unsigned int j = 0;

            LCNCE( lcn_ptr_array_create( &parr,
                                         (unsigned int) apr_hash_count( ph ),
                                         document_writer->posting_table_pool ));

            for( hi = apr_hash_first( document_writer->posting_table_pool, ph );
                 hi;
                 hi = apr_hash_next( hi ) )
            {
                void *val;
                apr_hash_this( hi, NULL, NULL, &val );
                parr->arr[ j++ ] = val;

#if 0
                lcn_posting_t *pst = (lcn_posting_t*) val;
                fprintf( stderr, "adding to list %s:%s\n",
                         lcn_term_field( pst->term ),
                         lcn_term_text( pst->term ));
#endif
            }

#if 0
            fprintf(stderr, "ADD TXT %s %d\n", lcn_term_text( ((lcn_posting_t*) (parr->arr[0]))->term ),
                    (int) parr->arr[0] );
            fprintf(stderr, "ADD TXT %s %d\n", lcn_term_text( ((lcn_posting_t*) (parr->arr[1]))->term ),
                    (int) parr->arr[1] );
#endif

            if ( parr->length > 1 )
            {
                qsort( parr->arr, parr->length, sizeof(lcn_posting_t*), lcn_document_writer_compare_postings );
            }

            document_writer->sorted_postings->arr[i] = parr;

#if 0
            for( j = 0; j < parr->length; j++ )
            {
                fprintf( stderr, "sorted %s\n", lcn_term_text( ((lcn_posting_t*) (parr->arr[j]))->term ));
            }
#endif
        }

        qsort( document_writer->sorted_postings->arr,
               document_writer->sorted_postings->length,
               sizeof(lcn_ptr_array_t*),
               lcn_document_writer_compare_postings_arrays );
    }
    while(0);

    return s;
}

static apr_status_t
lcn_document_writer_write_norms( lcn_document_writer_t *document_writer,
                                 const char *segment_name )
{
    apr_status_t s;
    lcn_index_output_t *norms = NULL;
    apr_pool_t *pool = NULL;

    do
    {
        unsigned int fi_size = lcn_field_infos_size( document_writer->field_infos );
        unsigned int i;
        lcn_field_info_t *field_info;

        LCNCE( apr_pool_create( &pool, document_writer->pool ) );

        for( i = 0; i < fi_size; i++ )
        {
            LCNCE( lcn_field_infos_nth_info( document_writer->field_infos,
                                             &field_info,
                                             i ));

            if ( lcn_field_info_is_indexed( field_info ) &&
                ! lcn_field_info_omit_norms( field_info ) )
            {
                float norm =
                    document_writer->field_boosts[ i ] *
                    lcn_similarity_length_norm( document_writer->similarity,
                                                lcn_field_info_name( field_info ),
                                                document_writer->field_lengths[ i ] );

                LCNCE( lcn_directory_create_output( document_writer->directory,
                                                    &norms,
                                                    apr_pstrcat( pool, segment_name, ".f", apr_itoa( pool, i ), NULL ),
                                                    pool ));

                LCNCE( lcn_ostream_write_byte( norms, lcn_similarity_encode_norm( document_writer->similarity, norm ) ));
            }

            if ( NULL != norms )
            {
                apr_status_t st = lcn_ostream_close( norms );
                s = s ? s : st;
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

static apr_status_t
lcn_document_writer_write_postings( lcn_document_writer_t *document_writer,
                                    const char *segment_name )
{
    apr_status_t s;
    apr_status_t s_save;

    apr_pool_t *pool = NULL;
    lcn_index_output_t *freq = NULL;
    lcn_index_output_t *prox = NULL;
    lcn_term_infos_writer_t *tis = NULL;

    /* TODO: TermVectorsWriter termVectorWriter = null */

    do
    {
        lcn_term_info_t ti;
        unsigned int field_ctr;
        unsigned int i;
        unsigned int j;

        LCNCE( apr_pool_create( &pool, document_writer->posting_table_pool ) );

        LCNCE( lcn_directory_create_output( document_writer->directory,
                                            &freq,
                                            apr_pstrcat( pool, segment_name, ".frq", NULL ),
                                            pool ));

        LCNCE( lcn_directory_create_output( document_writer->directory,
                                            &prox,
                                            apr_pstrcat( pool, segment_name, ".prx", NULL ),
                                            pool ));

        LCNCE( lcn_term_infos_writer_create( &tis,
                                             document_writer->directory,
                                             segment_name,
                                             document_writer->term_index_interval,
                                             pool ));

        for( field_ctr = 0;
             field_ctr < document_writer->sorted_postings->length;
             field_ctr++ )
        {
            unsigned int field_number;
            unsigned int *positions;
            lcn_field_info_t *field_info;
            unsigned int last_position;

            lcn_ptr_array_t *postings = (lcn_ptr_array_t*) document_writer->sorted_postings->arr[ field_ctr ];

            if ( 0 == postings->length )
            {
                continue;
            }

            LCNCM( lcn_field_infos_field_number( document_writer->field_infos,
                                                 &field_number,
                                                 lcn_term_field( ((lcn_posting_t*) postings->arr[0])->term ) ),
                   lcn_term_field( ((lcn_posting_t*) postings->arr[0])->term ) );

            for( i = 0; i < postings->length; i++ )
            {
                lcn_posting_t *posting = (lcn_posting_t*) postings->arr[ i ];
                unsigned int posting_freq = posting->freq;

                /* add an entry to the dictionary with pointers to prox and freq files */
                ti.doc_freq = 1;
                ti.freq_pointer = lcn_ostream_get_file_pointer( freq );
                ti.prox_pointer = lcn_ostream_get_file_pointer( prox );
                ti.skip_offset  = -1;

                LCNCE( lcn_term_infos_writer_add_term ( tis,
                                                        posting->term,
                                                        &ti,
                                                        field_number ));

                /* add an entry to the freq file */

                if ( 1 == posting_freq )  /* optimize freq == 1 */
                {
                    LCNCE( lcn_ostream_write_vint( freq, 1 )); /* set low bit of doc num */
                }
                else
                {
                    LCNCE( lcn_ostream_write_vint( freq, 0 ));              /* the document number */
                    LCNCE( lcn_ostream_write_vint( freq, posting_freq ));   /* frequency in doc    */
                }

                last_position = 0;
                positions = posting->positions;

                for( j = 0; j < posting_freq; j++ )  /* use delta-encoding */
                {
                    unsigned int position = positions[j];
                    LCNCE( lcn_ostream_write_vint( prox, position - last_position ));
                    last_position = position;
                }

#if 0
        if (termVectorWriter != null && termVectorWriter.isFieldOpen()) {
            termVectorWriter.addTerm(posting.term.text(), postingFreq, posting.positions, posting.offsets);
        }
#endif

        LCNCE( lcn_field_infos_by_number ( document_writer->field_infos,
                                           &field_info,
                                           field_number ));

        if ( LCN_TRUE == lcn_field_info_store_term_vector( field_info ) )
        {
            fprintf(stderr, "TODO: store term_vector\n" );
#if 0
            if (termVectorWriter == null) {
              termVectorWriter =
                new TermVectorsWriter(directory, segment, fieldInfos);
              termVectorWriter.openDocument();
            }
            termVectorWriter.openField(currentField);
#endif
        }
#if 0
        else if (termVectorWriter != null) {
            termVectorWriter.closeField();
        }
#endif
            }
        }

#if 0
      if (termVectorWriter != null)
        termVectorWriter.closeDocument();
#endif

    }
    while(0);

    /*
     * make an effort to close all streams we can but remember and re-throw
     * the first exception encountered in this process
     */

    if ( NULL != freq )
    {
        s_save = lcn_ostream_close( freq );
        s = ( s ? s : s_save );
    }

    if ( NULL != prox )
    {
        s_save = lcn_ostream_close( prox );
        s = ( s ? s : s_save );
    }

    if ( NULL != tis )
    {
        s_save = lcn_term_infos_writer_close( tis );
        s = ( s ? s : s_save );
    }

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    /* TODO if (termVectorWriter  != null)
     * try {  termVectorWriter.close(); } catch (IOException e) { if (keep == null) keep = e; }
     */

    return (apr_status_t) s;
}

apr_status_t
lcn_document_writer_add_document( lcn_document_writer_t *document_writer,
                                  const char *segment_name,
                                  lcn_document_t *document )
{
    apr_status_t s;

    do
    {
        lcn_fields_writer_t *fields_writer;
        char *fi_file_name;
        apr_status_t stat;

        /* write field names */

        LCNCE( lcn_field_infos_create( &(document_writer->field_infos), document_writer->pool ) );
        LCNCE( lcn_field_infos_add_document( document_writer->field_infos, document ) );

        LCNPV( fi_file_name = apr_pstrcat( document_writer->pool, segment_name, ".fnm", NULL ),
               APR_ENOMEM );

        LCNCE( lcn_field_infos_write( document_writer->field_infos, document_writer->directory, fi_file_name ));

        /* write field values */

        LCNCE( lcn_fields_writer_create( &fields_writer,
                                         document_writer->directory,
                                         segment_name,
                                         document_writer->field_infos,
                                         document_writer->pool ));

        s = lcn_fields_writer_add_document( fields_writer, document );
        stat = lcn_fields_writer_close( fields_writer );

        s = s ? s : stat;

        if ( s )
        {
            break;
        }

        /* invert doc into posting table */

        LCNCE( lcn_document_writer_init_posting_table( document_writer,
               lcn_field_infos_size( document_writer->field_infos ),
               lcn_document_get_boost( document )) );

        LCNCE( lcn_document_writer_invert_document( document_writer, document ));
        LCNCE( lcn_document_writer_sort_postings_table( document_writer ));

#if 0
        /*
          for (int i = 0; i < postings.length; i++) {
          Posting posting = postings[i];
          System.out.print(posting.term);
          System.out.print(" freq=" + posting.freq);
          System.out.print(" pos=");
          System.out.print(posting.positions[0]);
          for (int j = 1; j < posting.freq; j++)
          System.out.print("," + posting.positions[j]);
          System.out.println("");
          }
        */
#endif

        LCNCE( lcn_document_writer_write_postings( document_writer, segment_name ) );
        LCNCE( lcn_document_writer_write_norms( document_writer, segment_name ) );
    }
    while(0);

    return s;
}
