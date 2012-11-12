#include "lucene.h"
#include "term_docs.h"
#include "term_enum.h"
#include "index_reader.h"

/* {{{ static apr_status_t lcn_segment_term_docs_seek_term_info */

static apr_status_t
lcn_segment_term_docs_seek_term_info( lcn_term_docs_t *term_docs,
                                      lcn_term_info_t *term_info )
{
    apr_status_t s;

    do
    {
        LCNASSERT( NULL != term_info, LCN_ERR_NULL_PTR );

        term_docs->count = 0;
        term_docs->df = term_info->doc_freq;

        term_docs->doc = 0;
        term_docs->skip_doc = 0;
        term_docs->skip_count = 0;
        term_docs->num_skips = term_docs->df / term_docs->skip_interval;

        term_docs->freq_pointer = term_info->freq_pointer;
        term_docs->prox_pointer = term_info->prox_pointer;
        term_docs->skip_pointer = term_info->freq_pointer + term_info->skip_offset;

        LCNCE( lcn_istream_seek( term_docs->freq_stream, term_docs->freq_pointer ));

        term_docs->have_skipped = LCN_FALSE;
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_docs_seek_term_enum */

static apr_status_t
lcn_segment_term_docs_seek_term_enum( lcn_term_docs_t *term_docs,
                                      lcn_term_enum_t *term_enum )
{
    apr_status_t s = APR_SUCCESS;
    lcn_term_info_t *term_info;

    /* use comparison of fieldinfos to verify that termEnum */
    /* belongs to the same segment as this SegmentTermDocs  */

    if ( term_enum->field_infos == ((lcn_segment_reader_t*)term_docs->parent)->field_infos )
    {
        /* optimized case */
        term_info = term_enum->term_info;
    }
    else
    {
        /* punt case */
        LCNCR( lcn_term_infos_reader_get_by_term( ((lcn_segment_reader_t*)term_docs->parent)->tis,
                                                  &term_info,
                                                  term_enum->term ) );
    }

    LCNCR( term_docs->seek_term_info( term_docs, term_info ));

    return s;
}

/* }}} */

static apr_status_t
lcn_segment_term_docs_seek_term( lcn_term_docs_t *term_docs,
                                 const lcn_term_t *term )
{
    apr_status_t s;
    lcn_term_info_t *term_info;

    if(( s = lcn_term_infos_reader_get_by_term( ((lcn_segment_reader_t*)term_docs->parent)->tis,
                                                &term_info,
                                                term )))
    {
        return s;
    }

    LCNCR( term_docs->seek_term_info( term_docs, term_info ));

    return s;
}

/* {{{ static apr_status_t lcn_segment_term_docs_close */

static apr_status_t
lcn_segment_term_docs_close( lcn_term_docs_t *term_docs )
{
    apr_status_t s;

    LCNCR( lcn_istream_close( term_docs->freq_stream ));

    if ( NULL != term_docs->skip_stream )
    {
        LCNCR( lcn_istream_close( term_docs->skip_stream ) );
    }

    return s;
}

/* }}} */

/* {{{ static unsigned int lcn_segment_term_docs_doc */

static unsigned int
lcn_segment_term_docs_doc( lcn_term_docs_t *term_docs )
{
    return term_docs->doc;
}

/* }}} */

/* {{{ static unsigned int lcn_segment_term_docs_freq */

static unsigned int
lcn_segment_term_docs_freq( lcn_term_docs_t *term_docs )
{
    return term_docs->freq;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_docs_nop */

static apr_status_t
lcn_segment_term_docs_nop( lcn_term_docs_t *term_docs )
{
    return APR_SUCCESS;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_skipping_doc */

static apr_status_t
lcn_segment_term_positions_skipping_doc( lcn_term_docs_t *term_positions )
{
    apr_status_t s = APR_SUCCESS;

    int f;
    unsigned int pos;

    for ( f = term_positions->freq; f > 0; f-- ) /* skip all positions */
    {
        LCNCE( lcn_istream_read_vint( term_positions->prox_stream, &pos ) );
    }

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_docs_skip_prox */

static apr_status_t
lcn_segment_term_docs_skip_prox( lcn_term_docs_t *term_docs, apr_off_t prox_pointer )
{
    return APR_SUCCESS;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_docs_next */

static apr_status_t
lcn_segment_term_docs_next( lcn_term_docs_t *term_docs )
{
    apr_status_t s = APR_SUCCESS;

    while(1)
    {
        unsigned int doc_code;

        if ( term_docs->count == term_docs->df)
        {
            return LCN_ERR_ITERATOR_NO_NEXT;
        }

        LCNCE( lcn_istream_read_vint( term_docs->freq_stream, &doc_code ));

        term_docs->doc += ( doc_code >> 1);  /* shift off low bit */

        if (( doc_code & 1 ))                /* if low bit is set */
        {
            term_docs->freq = 1;             /* freq is one       */
        }
        else
        {                               
	    /* else read freq    */
            LCNCE( lcn_istream_read_vint( term_docs->freq_stream,
                                          (unsigned int*)&(term_docs->freq) ));
        }

        term_docs->count++;

        if ( NULL == term_docs->deleted_docs  ||
             ! lcn_bitvector_get_bit( term_docs->deleted_docs, term_docs->doc ) )
        {
            break;
        }

        LCNCE( term_docs->skipping_doc( term_docs ) );
    }

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_next */

static apr_status_t
lcn_segment_term_positions_next( lcn_term_docs_t *term_positions )
{
    apr_status_t s = APR_SUCCESS;
    int f;
    unsigned int position;

    for( f = term_positions->prox_count; f > 0; f-- ) /* skip unread positions */
    {
        LCNCE( lcn_istream_read_vint( term_positions->prox_stream, &position ));
    }

    if ( s )
    {
        return s;
    }

    s = lcn_segment_term_docs_next( term_positions ); /* run super */

    if ( APR_SUCCESS == s )
    {
        term_positions->prox_count = term_positions->freq; /* note frequency */
        term_positions->position = 0;                      /* reset position */
    }

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_docs_read */

static apr_status_t
lcn_segment_term_docs_read( lcn_term_docs_t *term_docs,
                            lcn_int_array_t* docs,
                            lcn_int_array_t* freqs,
                            unsigned int *read_entries )
{
    apr_status_t s = APR_SUCCESS;
    int i = 0;

    while( i < docs->length &&
           term_docs->count < term_docs->df )
    {
        unsigned int doc_code;

        LCNCE( lcn_istream_read_vint( term_docs->freq_stream, &doc_code ));

        term_docs->doc += ( doc_code >> 1);  /* shift off low bit */

        if (( doc_code & 1 ))                /* if low bit is set */
        {
            term_docs->freq = 1;             /* freq is one       */
        }
        else                                 /* else read freq    */
        {
            LCNCE( lcn_istream_read_vint( term_docs->freq_stream,
                                          (unsigned int*)&(term_docs->freq) ));
        }

        term_docs->count++;

        if ( NULL == term_docs->deleted_docs  ||
             ! lcn_bitvector_get_bit( term_docs->deleted_docs, term_docs->doc ) )
        {

            docs->arr[i]  = term_docs->doc;
            freqs->arr[i] = term_docs->freq;

            ++i;
        }
    }

    *read_entries = (APR_SUCCESS == s ? i : 0 );

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_segment_term_docs_skip_to */

apr_status_t
lcn_segment_term_docs_skip_to( lcn_term_docs_t *term_docs, unsigned int target )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( term_docs->df >= term_docs->skip_interval ) /* optimized case */
        {
            /* lazily clone   */
            if ( NULL == term_docs->skip_stream )
            {
                LCNCE( lcn_istream_clone( term_docs->freq_stream,
                                          &(term_docs->skip_stream),
                                          term_docs->pool ) );
            }

            /* lazily seek skip stream */

            if ( ! term_docs->have_skipped )
            {
                LCNCE( lcn_istream_seek( term_docs->skip_stream,
                                         term_docs->skip_pointer ) );

                term_docs->have_skipped = LCN_TRUE;
            }

            /* scan skip data */
            {
                int last_skip_doc = term_docs->skip_doc;
                apr_off_t last_freq_pointer = lcn_istream_file_pointer( term_docs->freq_stream );
                apr_off_t last_prox_pointer = -1;
                int num_skipped = -1 - (term_docs->count % term_docs->skip_interval);

                while( target > term_docs->skip_doc )
                {
                    unsigned int vint;

                    last_skip_doc = term_docs->skip_doc;
                    last_freq_pointer = term_docs->freq_pointer;
                    last_prox_pointer = term_docs->prox_pointer;

                    if ( 0 != term_docs->skip_doc  &&
                         term_docs->skip_doc >= term_docs->doc )
                    {
                        num_skipped += term_docs->skip_interval;
                    }

                    if ( term_docs->skip_count >= term_docs->num_skips )
                    {
                        break;
                    }

                    LCNCE( lcn_istream_read_vint( term_docs->skip_stream, &vint ));
                    term_docs->skip_doc += vint;

                    LCNCE( lcn_istream_read_vint( term_docs->skip_stream, &vint ));
                    term_docs->freq_pointer += vint;

                    LCNCE( lcn_istream_read_vint( term_docs->skip_stream, &vint ));
                    term_docs->prox_pointer += vint;

                    term_docs->skip_count++;
                }

                if (s)
                {
                    break;
                }

                /* if we found something to skip, then skip it */

                if ( last_freq_pointer > lcn_istream_file_pointer( term_docs->freq_stream ) )
                {
                    LCNCE( lcn_istream_seek( term_docs->freq_stream, last_freq_pointer ));
                    LCNCE( term_docs->skip_prox( term_docs, last_prox_pointer ) );

                    term_docs->doc = last_skip_doc;
                    term_docs->count += num_skipped;
                }
            }
        }

        if ( s )
        {
            break;
        }

        do
        {
            s = term_docs->next( term_docs );

            if ( LCN_ERR_ITERATOR_NO_NEXT == s )
            {
                return LCN_ERR_NO_SUCH_DOC;
            }

            if ( s )
            {
                break;
            }
        }
        while( target > term_docs->doc );

    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_nop */

static apr_status_t
lcn_segment_term_positions_nop( lcn_term_docs_t *term_positions,
                                apr_ssize_t *position )
{
    return LCN_ERR_UNSUPPORTED_OPERATION;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_next_position */

static apr_status_t
lcn_segment_term_positions_next_position( lcn_term_docs_t *term_positions,
                                          apr_ssize_t *position )
{
    apr_status_t s;
    unsigned int pos;

    term_positions->prox_count--;
    LCNCR( lcn_istream_read_vint( term_positions->prox_stream,
                                  (unsigned int*)&pos ) );

    term_positions->position += pos;
    *position = term_positions->position;

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_segment_term_docs_create */

/**
 * @param index_reader MUST be a segment reader
 */
apr_status_t
lcn_segment_term_docs_create( lcn_index_reader_t *index_reader,
                              lcn_term_docs_t **term_docs,
                              apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_segment_reader_t *segment_reader = (lcn_segment_reader_t*) index_reader;

        LCNPV( *term_docs = (lcn_term_docs_t*) apr_pcalloc( pool, sizeof(lcn_term_docs_t) ),
               APR_ENOMEM );

        (*term_docs)->parent = index_reader;
        (*term_docs)->pool   = pool;

        LCNCE( lcn_istream_clone( segment_reader->freq_stream,
                                  &((*term_docs)->freq_stream),
                                  pool ));
        (*term_docs)->deleted_docs   = segment_reader->deleted_docs;
        (*term_docs)->skip_interval  = lcn_term_infos_reader_skip_interval( segment_reader->tis );

        (*term_docs)->seek_term      = lcn_segment_term_docs_seek_term;
        (*term_docs)->seek_term_info = lcn_segment_term_docs_seek_term_info;
        (*term_docs)->close          = lcn_segment_term_docs_close;
        (*term_docs)->seek_term_enum = lcn_segment_term_docs_seek_term_enum;
        (*term_docs)->get_doc        = lcn_segment_term_docs_doc;
        (*term_docs)->get_freq       = lcn_segment_term_docs_freq;
        (*term_docs)->next           = lcn_segment_term_docs_next;
        (*term_docs)->skipping_doc   = lcn_segment_term_docs_nop;
        (*term_docs)->skip_prox      = lcn_segment_term_docs_skip_prox;
        (*term_docs)->skip_to        = lcn_segment_term_docs_skip_to;
        (*term_docs)->read           = lcn_segment_term_docs_read;
        (*term_docs)->next_position  = lcn_segment_term_positions_nop;
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_read */

apr_status_t
lcn_term_docs_read( lcn_term_docs_t *term_docs,
                    lcn_int_array_t* docs,
                    lcn_int_array_t* freqs,
                    unsigned int *read_entries )
{
    return term_docs->read( term_docs, docs, freqs, read_entries );
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_skip_to */

apr_status_t
lcn_term_docs_skip_to( lcn_term_docs_t *term_docs,
                       unsigned int target )
{
    return term_docs->skip_to( term_docs, target );
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_seek_term */

apr_status_t
lcn_term_docs_seek_term( lcn_term_docs_t *term_docs,
                         const lcn_term_t *term )
{
    return term_docs->seek_term( term_docs, term );
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_seek_term_enum */

apr_status_t
lcn_term_docs_seek_term_enum( lcn_term_docs_t *term_docs,
                              lcn_term_enum_t *term_enum )
{
    return term_docs->seek_term_enum( term_docs, term_enum );
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_close */

apr_status_t
lcn_term_docs_close( lcn_term_docs_t *term_docs )
{
    return term_docs->close( term_docs );
}

/* }}} */

/* {{{ unsigned int lcn_term_docs_doc */

unsigned int
lcn_term_docs_doc( lcn_term_docs_t *term_docs )
{
    return term_docs->get_doc( term_docs );
}

/* }}} */

/* {{{ unsigned int lcn_term_docs_freq */

unsigned int
lcn_term_docs_freq( lcn_term_docs_t *term_docs )
{
    return term_docs->get_freq( term_docs );
}

/* }}} */

/* {{{ apr_status_t lcn_term_docs_next */

apr_status_t
lcn_term_docs_next( lcn_term_docs_t *term_docs )
{
    return term_docs->next( term_docs );
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_seek_term_info */

static apr_status_t
lcn_segment_term_positions_seek_term_info( lcn_term_docs_t *term_positions, lcn_term_info_t *term_info )
{
    apr_status_t s;

    LCNCR( lcn_segment_term_docs_seek_term_info( term_positions, term_info ) );
    LCNCR( lcn_istream_seek( term_positions->prox_stream, term_info->prox_pointer ) );
    term_positions->prox_count = 0;

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_close */

static apr_status_t
lcn_segment_term_positions_close( lcn_term_docs_t *term_positions )
{
    apr_status_t s;

    LCNCR( lcn_segment_term_docs_close( term_positions ) );
    LCNCR( lcn_istream_close( term_positions->prox_stream ) );

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_term_positions_next_position */

apr_status_t
lcn_term_positions_next_position( lcn_term_docs_t *term_positions,
                                  apr_ssize_t *position )
{
    return term_positions->next_position( term_positions, position );
}

/* }}} */

/* {{{ static apr_status_t lcn_segment_term_positions_skip_prox */

static apr_status_t
lcn_segment_term_positions_skip_prox (lcn_term_docs_t *term_positions, apr_off_t prox_pointer )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_istream_seek( term_positions->prox_stream, prox_pointer ) );
        term_positions->prox_count = 0;
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_segment_term_positions_read */

apr_status_t
lcn_segment_term_positions_read( lcn_term_docs_t *term_docs,
                                 lcn_int_array_t* docs,
                                 lcn_int_array_t* freqs,
                                 unsigned int *read_entries )
{
    return LCN_ERR_UNSUPPORTED_OPERATION;
}

/* }}} */

/* {{{ apr_status_t lcn_segment_term_positions_create */

/**
 * @param index_reader MUST be a segment reader
 */
apr_status_t
lcn_segment_term_positions_create( lcn_index_reader_t *index_reader,
                                   lcn_term_docs_t **term_positions,
                                   apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_segment_term_docs_create( index_reader, term_positions, pool ) );
    LCNCR( lcn_istream_clone( ((lcn_segment_reader_t*)index_reader)->prox_stream,
                              &((*term_positions)->prox_stream),
                              pool ));

    (*term_positions)->seek_term_info = lcn_segment_term_positions_seek_term_info;
    (*term_positions)->close          = lcn_segment_term_positions_close;
    (*term_positions)->next           = lcn_segment_term_positions_next;
    (*term_positions)->skipping_doc   = lcn_segment_term_positions_skipping_doc;
    (*term_positions)->skip_prox      = lcn_segment_term_positions_skip_prox;
    (*term_positions)->read           = lcn_segment_term_positions_read;
    (*term_positions)->next_position  = lcn_segment_term_positions_next_position;

    return s;
}

/* }}} */

/******************************************************************************
 *
 *                           MultiTermDocs implementation
 *
 ******************************************************************************/

/* {{{ apr_status_t lcn_multi_term_docs_skip_to */

apr_status_t
lcn_multi_term_docs_skip_to( lcn_term_docs_t *term_docs, unsigned int target )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        s = lcn_term_docs_next( term_docs );

        if ( LCN_ERR_ITERATOR_NO_NEXT == s )
        {
            return LCN_ERR_NO_SUCH_DOC;
        }

        if ( s )
        {
            break;
        }
    }
    while( target > lcn_term_docs_doc( term_docs ) );

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_get_term_docs_by_index */

static apr_status_t
lcn_multi_term_docs_get_term_docs_by_index( lcn_term_docs_t *term_docs,
                                            lcn_term_docs_t **current,
                                            int i,
                                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_term_docs_t *tdocs;

        LCNPV( term_docs->term, LCN_ERR_TERM_DOCS_TERM_IS_NULL );

        if ( NULL == (tdocs = term_docs->reader_term_docs[ i ]))
        {
            LCNCE( term_docs->term_docs( &tdocs, term_docs->readers[ i ], pool ));
            term_docs->reader_term_docs[i] = tdocs;
        }

        *current = tdocs;

        s = lcn_term_docs_seek_term( tdocs, term_docs->term );

        if ( LCN_ERR_SCAN_ENUM_NO_MATCH == s )
        {
            s = APR_SUCCESS;
        }
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_read */

static apr_status_t
lcn_multi_term_docs_read( lcn_term_docs_t *term_docs,
                          lcn_int_array_t* docs,
                          lcn_int_array_t* freqs,
                          unsigned int *read_entries )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *pool = NULL;

    while(1)
    {
        unsigned int end;

        while( NULL == term_docs->current )
        {
            lcn_bool_t break_cond = LCN_FALSE;

            if ( term_docs->pointer < term_docs->readers_count ) /* try next segment */
            {
                if ( NULL == pool )
                {
                    LCNCE( apr_pool_create( &pool, term_docs->pool ));
                }

                term_docs->base = term_docs->starts[ term_docs->pointer ];

                s = lcn_multi_term_docs_get_term_docs_by_index( term_docs,
                                                                &(term_docs->current),
                                                                term_docs->pointer++,
                                                                pool );
            }
            else
            {
                break_cond = LCN_TRUE;
            }

            if ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ||
                break_cond )
            {
                if ( term_docs->pointer >= term_docs->readers_count )
                {
                    *read_entries = 0;
                    return APR_SUCCESS;
                }
                else
                {
                    s = APR_SUCCESS;
                }
            }
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_term_docs_read( term_docs->current,
                                   docs,
                                   freqs,
                                   &end ));

        if ( end == 0 ) /* none left in segment */
        {
            term_docs->current = NULL;
        }
        else /* got some */
        {
            unsigned int b = term_docs->base;
            unsigned int i;

            for( i = 0; i < end; i++ )
            {
                int d = docs->arr[i];
                docs->arr[i] = ( b+d );
            }

            *read_entries = end;
            break;
        }
    }

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_close */

static apr_status_t
lcn_multi_term_docs_close( lcn_term_docs_t *term_docs )
{
    apr_status_t s = APR_SUCCESS;
    int i;

    for( i = 0; i < term_docs->readers_count; i++ )
    {
        if ( NULL != term_docs->reader_term_docs[i] )
        {
            apr_status_t stat = lcn_term_docs_close( term_docs->reader_term_docs[i] );

            if ( APR_SUCCESS == s )
            {
                s = stat;
            }
        }
    }

    return s;
}

/* }}} */

/* {{{ static unsigned int lcn_multi_term_docs_freq */

static unsigned int
lcn_multi_term_docs_freq( lcn_term_docs_t *term_docs )
{
    return lcn_term_docs_freq( term_docs->current );
}

/* }}} */

/* {{{ static unsigned int lcn_multi_term_docs_doc */

static unsigned int
lcn_multi_term_docs_doc( lcn_term_docs_t *term_docs )
{
    return term_docs->base + lcn_term_docs_doc( term_docs->current );
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_seek_term */

static apr_status_t
lcn_multi_term_docs_seek_term( lcn_term_docs_t *term_docs,
                               const lcn_term_t *term )
{
    term_docs->term = term;
    term_docs->base = 0;
    term_docs->pointer = 0;
    term_docs->current = NULL;

    return APR_SUCCESS;
}

static apr_status_t
lcn_multi_term_docs_seek_term_enum( lcn_term_docs_t *term_docs,
                                    lcn_term_enum_t *term_enum )
{
    return lcn_multi_term_docs_seek_term( term_docs, lcn_term_enum_term( term_enum ) );
}


/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_get_term_docs_by_reader */

static apr_status_t
lcn_multi_term_docs_get_term_docs_by_reader( lcn_term_docs_t **current,
                                             lcn_index_reader_t *reader,
                                             apr_pool_t *pool )
{
    return lcn_index_reader_term_docs( reader, current, pool );
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_docs_next */

static apr_status_t
lcn_multi_term_docs_next( lcn_term_docs_t *term_docs )
{
    apr_status_t s = APR_SUCCESS;

    if ( NULL != term_docs->current )
    {
        if ( APR_SUCCESS == ( s = lcn_term_docs_next( term_docs->current ) ))
        {
            return s;
        }
    }

    do
    {
        if ( term_docs->pointer < term_docs->readers_count )
        {
            term_docs->base = term_docs->starts[ term_docs->pointer ];

            s = lcn_multi_term_docs_get_term_docs_by_index( term_docs,
                                                            &(term_docs->current),
                                                            term_docs->pointer++,
                                                            term_docs->pool );

            if ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ||
                 LCN_ERR_TERM_DOCS_TERM_IS_NULL == s )
            {
                term_docs->current = NULL;
            }

            s = lcn_multi_term_docs_next( term_docs );
        }
        else
        {
            s = LCN_ERR_ITERATOR_NO_NEXT;
        }
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ apr_status_t lcn_multi_term_docs_create */

apr_status_t
lcn_multi_term_docs_create( lcn_term_docs_t **term_docs,
                            lcn_index_reader_t **readers,
                            unsigned int *starts,
                            int readers_count,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *term_docs = (lcn_term_docs_t*) apr_pcalloc(pool, sizeof(lcn_term_docs_t) ),
               APR_ENOMEM );

        LCNPV( (*term_docs)->reader_term_docs = (lcn_term_docs_t**) apr_pcalloc(pool, sizeof(lcn_term_docs_t*) * readers_count),
               APR_ENOMEM );

        (*term_docs)->pool    = pool;
        (*term_docs)->readers = readers;
        (*term_docs)->starts  = starts;
        (*term_docs)->readers_count = readers_count;

        (*term_docs)->next           = lcn_multi_term_docs_next;
        (*term_docs)->seek_term      = lcn_multi_term_docs_seek_term;
        (*term_docs)->seek_term_enum = lcn_multi_term_docs_seek_term_enum;
        (*term_docs)->get_doc        = lcn_multi_term_docs_doc;
        (*term_docs)->get_freq       = lcn_multi_term_docs_freq;
        (*term_docs)->close          = lcn_multi_term_docs_close;
        (*term_docs)->term_docs      = lcn_multi_term_docs_get_term_docs_by_reader;
        (*term_docs)->read           = lcn_multi_term_docs_read;
        (*term_docs)->skip_to        = lcn_multi_term_docs_skip_to;
    }
    while(0);

    return s;
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_positions_next_position */

static apr_status_t
lcn_multi_term_positions_next_position( lcn_term_docs_t *term_positions,
                                        apr_ssize_t *position )
{
    return term_positions->current->next_position( term_positions->current, position );
}

/* }}} */

/* {{{ static apr_status_t lcn_multi_term_positions_term_docs */

static apr_status_t
lcn_multi_term_positions_term_docs( lcn_term_docs_t **term_positions,
                                    lcn_index_reader_t *index_reader,
                                    apr_pool_t *pool )
{
    return lcn_index_reader_term_positions( index_reader, term_positions, pool );
}

/* }}} */

/* {{{ apr_status_t lcn_multi_term_positions_create */

apr_status_t
lcn_multi_term_positions_create(  lcn_term_docs_t **term_positions,
                                  lcn_index_reader_t **readers,
                                  unsigned int *starts,
                                  int readers_count,
                                  apr_pool_t *pool )
{
    apr_status_t s;

    LCNCR( lcn_multi_term_docs_create( term_positions, readers, starts, readers_count, pool ) );

    (*term_positions)->next_position = lcn_multi_term_positions_next_position;
    (*term_positions)->term_docs     = lcn_multi_term_positions_term_docs;

    return s;
}

/* }}} */



