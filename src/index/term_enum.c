#include "lucene.h"
#include "lcn_term_enum.h"
#include "lcn_segment_merge_info.h"
#include "segment_infos.h"
#include "term_enum.h"
#include "term_infos_writer.h"

#define SEGMENT_TERM_ENUM_INITIAL_BUFFER_SIZE 200

/**
 * Methods which were not ported to C an may be obsolete:
 *
 * prev()
 * termInfo()
 * termInfo(TermInfo)
 * docFreq()
 * freqPointer()
 * proxPointer()
 *
 */

static apr_status_t
lcn_term_enum_grow_buffer( lcn_term_enum_t *term_enum, unsigned int len )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        if ( term_enum->buffer_size < len )
        {
            char *term_text;
            char *prev_text;

            apr_pool_t *save_pool = term_enum->save_pool;

            LCNPV( term_text = (char*) apr_pcalloc( save_pool, len * sizeof(char) ),
                   APR_ENOMEM );
            LCNPV( prev_text = (char*) apr_pcalloc( save_pool, len * sizeof(char) ),
                   APR_ENOMEM );

            apr_cpystrn( term_text, term_enum->term_text, term_enum->buffer_size );
            apr_cpystrn( prev_text, term_enum->prev_text, term_enum->buffer_size );

            term_enum->save_pool = term_enum->buffer_pool;
            term_enum->buffer_pool = save_pool;

            apr_pool_clear( term_enum->save_pool );

            term_enum->term_text = term_text;
            term_enum->prev_text = prev_text;

            term_enum->term->text = term_text;
            term_enum->prev->text = prev_text;

            term_enum->buffer_size = len;
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_term_enum_clone( lcn_term_enum_t *term_enum,
                     lcn_term_enum_t **clone,
                     apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_index_input_t *istream;

        LCNCE( lcn_index_input_clone( term_enum->istream, &istream, pool ) );

        LCNCE( lcn_segment_term_enum_create( clone, istream, term_enum->field_infos,
                                             term_enum->is_index, pool ) );

        LCNCE( lcn_term_enum_grow_buffer( *clone, term_enum->buffer_size) );

        apr_cpystrn( (*clone)->term->text, term_enum->term->text, term_enum->buffer_size - 1 );
        (*clone)->term->field = term_enum->term->field;

        apr_cpystrn( (*clone)->prev->text, term_enum->prev->text, term_enum->buffer_size - 1);

        (*clone)->prev->field = term_enum->prev->field;
        *((*clone)->term_info) = *(term_enum->term_info);

        (*clone)->is_clone = LCN_TRUE;
        (*clone)->position = term_enum->position;

        LCNCE( lcn_term_enum_seek( *clone, lcn_index_input_file_pointer( term_enum->istream ),
                                   (*clone)->position, term_enum->term, term_enum->term_info ) );
    }
    while(0);

    return s;
}


apr_status_t
lcn_term_enum_seek( lcn_term_enum_t *term_enum,
                    apr_off_t pointer,
                    apr_off_t p,
                    lcn_term_t *term,
                    lcn_term_info_t *ti )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_index_input_seek( term_enum->istream, pointer ) );
        term_enum->position = p;

        LCNCE( lcn_term_enum_grow_buffer( term_enum, strlen( term->text ) + 1 ));
        apr_cpystrn( term_enum->term->text, term->text, term_enum->buffer_size - 1 );
        term_enum->term->field = term->field;

        /* whe express the condition termEnum.prev == null in java        */
        /* as term_enum->prev->field == NULL to keep the term_enum->prev  */
        /* to avoid new allocation of term_enum->prev. Take into account, */
        /* that the field attributes are atoms                            */

        term_enum->prev->field = NULL;
        *(term_enum->term_info) = *ti;
    }
    while(0);

    return s;
}

apr_uint64_t
lcn_term_enum_size( lcn_term_enum_t *term_enum )
{
    return term_enum->size;
}

static unsigned int
lcn_segment_term_enum_doc_freq( lcn_term_enum_t *term_enum )
{
    return term_enum->term_info->doc_freq;
}

static apr_status_t
lcn_segment_term_enum_close( lcn_term_enum_t *term_enum )
{
    if ( NULL != term_enum->save_pool )
    {
        apr_pool_destroy( term_enum->save_pool );
    }

    if ( NULL != term_enum->buffer_pool )
    {
        apr_pool_destroy( term_enum->buffer_pool );
    }

    return lcn_index_input_close( term_enum->istream );
}

const lcn_term_t *
lcn_term_enum_term( lcn_term_enum_t *term_enum )
{
    return term_enum->term;
}

static apr_status_t
lcn_term_enum_read_term( lcn_term_enum_t *term_enum )
{
    apr_status_t s;

    do
    {
        unsigned int start;
        unsigned int length;
        unsigned int total_length;
        unsigned int field_number;

        LCNCE( lcn_index_input_read_vint( term_enum->istream, &start ) );
        LCNCE( lcn_index_input_read_vint( term_enum->istream, &length ) );
        total_length = start + length;

        if ( term_enum->buffer_size < total_length + 1 )
        {
            LCNCE( lcn_term_enum_grow_buffer( term_enum, total_length + 1 ) );
        }

        LCNCE( lcn_index_input_read_chars( term_enum->istream, term_enum->term_text, start, length ) );
        term_enum->term_text[total_length] = '\0';

        LCNCE( lcn_index_input_read_vint( term_enum->istream, &field_number ) );

        /* first term info in index term infos file has field number -1              */
        /* we check this here instead of letting lcn_field_infos_name_by_number      */
        /* act in a special way. we do not assume some default values on nonexisting */
        /* field index numbers.                                                      */

        if ( -1 != field_number )
        {
            LCNCE( lcn_field_infos_name_by_number( term_enum->field_infos,
                                                   (char**) &(term_enum->term->field),
                                                   field_number ) );
        }
        else
        {
            term_enum->term->field = lcn_atom_get_str ( "" );
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_segment_term_enum_next( lcn_term_enum_t *term_enum )
{
    apr_status_t s = APR_SUCCESS;

    if ( LCN_TRUE == term_enum->skip_first_next )
    {
        term_enum->skip_first_next = LCN_FALSE;
        return APR_SUCCESS;
    }

    do
    {
        unsigned int i;

        if ( term_enum->position++ >= ((apr_off_t)term_enum->size) - 1 )
        {
            s = LCN_ERR_ITERATOR_NO_NEXT;
            break;
        }

        term_enum->prev->field = term_enum->term->field;
        apr_cpystrn( term_enum->prev->text, term_enum->term->text, term_enum->buffer_size - 1 );

        LCNCE( lcn_term_enum_read_term( term_enum ) );

        /* read doc freq     */
        LCNCE( lcn_index_input_read_vint( term_enum->istream, &(term_enum->term_info->doc_freq)  ));

        /* read freq pointer */
        LCNCE( lcn_index_input_read_vint( term_enum->istream, &i ) );
        term_enum->term_info->freq_pointer += i;

        /* read prox pointer */
        LCNCE( lcn_index_input_read_vint( term_enum->istream, &i ) );
        term_enum->term_info->prox_pointer += i;

        if ( term_enum->term_info->doc_freq >= term_enum->skip_interval )
        {
            unsigned int skip_offset;
            LCNCE( lcn_index_input_read_vint( term_enum->istream, &skip_offset ) );
            term_enum->term_info->skip_offset = (apr_off_t) skip_offset;
        }

        /* read index ponter */
        if ( term_enum->is_index )
        {
            LCNCE( lcn_index_input_read_vint( term_enum->istream, &i ) );
            term_enum->index_pointer += i;
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_base_term_enum_create( lcn_term_enum_t **term_enum, apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *term_enum = apr_pcalloc( pool, sizeof( lcn_term_enum_t )),
               APR_ENOMEM );

        (*term_enum)->pool = pool;

        LCNCE( apr_pool_create( &((*term_enum)->save_pool), pool ) );
        LCNCE( apr_pool_create( &((*term_enum)->buffer_pool), pool ) );

        LCNPV( (*term_enum)->term_text = apr_pcalloc( (*term_enum)->buffer_pool,
                       SEGMENT_TERM_ENUM_INITIAL_BUFFER_SIZE * sizeof(char) ),
               APR_ENOMEM );

        LCNPV( (*term_enum)->prev_text = apr_pcalloc( (*term_enum)->buffer_pool,
                        SEGMENT_TERM_ENUM_INITIAL_BUFFER_SIZE * sizeof(char) ),
               APR_ENOMEM );

        (*term_enum)->buffer_size = SEGMENT_TERM_ENUM_INITIAL_BUFFER_SIZE;

        LCNCE( lcn_term_create( &((*term_enum)->term), "", (*term_enum)->term_text, LCN_TERM_NO_TEXT_COPY, pool ) );
        LCNCE( lcn_term_create( &((*term_enum)->prev), "", (*term_enum)->prev_text, LCN_TERM_NO_TEXT_COPY, pool ) );

        (*term_enum)->is_clone = LCN_FALSE;
        (*term_enum)->skip_first_next = LCN_FALSE;
    }
    while(0);

    return s;
}

apr_status_t
lcn_segment_term_enum_create( lcn_term_enum_t **term_enum,
                              lcn_index_input_t *istream,
                              lcn_field_infos_t *field_infos,
                              lcn_bool_t is_index,
                              apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        int format;

        LCNCE( lcn_base_term_enum_create( term_enum, pool ));
        LCNPV( (*term_enum)->term_info = apr_pcalloc( (*term_enum)->pool, sizeof(lcn_term_info_t) ),
               APR_ENOMEM );

        (*term_enum)->istream = istream;
        (*term_enum)->field_infos = field_infos;
        (*term_enum)->is_index = is_index;
        (*term_enum)->position = -1;

        LCNCE( lcn_index_input_read_int( istream, &format ) );

        if ( format > 0 ) /* original-format file, without explicit format version number */
        {
            (*term_enum)->size = format;
            format = 0;

            /* back-compatible settings */

            (*term_enum)->index_interval = 128;
            (*term_enum)->skip_interval  = JAVA_INTEGER_MAX_VALUE; /* switch off skipTo optimization */
        }
        else /* we have a format version number */
        {
  	    int index_interval, skip_interval;

            LCNASSERT( format == LCN_TERM_INFOS_FILE_FORMAT, LCN_ERR_SEGMENT_INFOS_UNKNOWN_FILE_FORMAT );

            LCNCE( lcn_index_input_read_ulong( istream, &((*term_enum)->size) ) );
            LCNCE( lcn_index_input_read_int( istream, &index_interval) );
	    (*term_enum)->index_interval = index_interval;
            LCNCE( lcn_index_input_read_int( istream, &skip_interval ) );
	    (*term_enum)->skip_interval = skip_interval;
        }

        (*term_enum)->get_doc_freq = lcn_segment_term_enum_doc_freq;
        (*term_enum)->next         = lcn_segment_term_enum_next;
        (*term_enum)->close        = lcn_segment_term_enum_close;
    }
    while(0);

    return s;
}

unsigned int
lcn_term_enum_doc_freq( lcn_term_enum_t *term_enum )
{
    return term_enum->get_doc_freq( term_enum );
}

apr_status_t
lcn_term_enum_next( lcn_term_enum_t *term_enum )
{
    return term_enum->next( term_enum );
}

apr_status_t
lcn_term_enum_close( lcn_term_enum_t *term_enum )
{
    return term_enum->close( term_enum );
}

/****************************************************************************************
 *
 *                           SegmentMergeQueue implementation
 *
 ****************************************************************************************/

static lcn_bool_t
lcn_segment_merge_queue_less_than( lcn_priority_queue_t *pq, void* a, void*b )
{
    lcn_segment_merge_info_t *smiA = (lcn_segment_merge_info_t*) a;
    lcn_segment_merge_info_t *smiB = (lcn_segment_merge_info_t*) b;

    int comparison = lcn_term_compare( smiA->term, smiB->term );

    if ( 0 == comparison )
    {
        return smiA->base < smiB->base;
    }
    else
    {
        return comparison < 0;
    }
}

apr_status_t
lcn_segment_merge_queue_close( lcn_priority_queue_t *queue )
{
    apr_status_t s = APR_SUCCESS;
    apr_status_t tmp;

    while( NULL != lcn_priority_queue_top( queue ) )
    {
        lcn_segment_merge_info_t *smi = (lcn_segment_merge_info_t*) lcn_priority_queue_pop( queue );

        if (( tmp = lcn_segment_merge_info_close( smi )))
        {
            s = tmp;
        }
    }

    return s;
}


apr_status_t
lcn_segment_merge_queue_create( lcn_priority_queue_t** priority_queue,
                                unsigned int size,
                                apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_priority_queue_create ( priority_queue,
                                       lcn_segment_merge_queue_less_than,
                                       pool ) );
    LCNCR( lcn_priority_queue_initialize( *priority_queue, size ) );

    return s;
}

static unsigned int
lcn_multi_term_enum_doc_freq( lcn_term_enum_t *term_enum )
{
    return term_enum->doc_freq;
}

apr_status_t
lcn_multi_term_enum_next( lcn_term_enum_t *term_enum )
{
    apr_status_t s = APR_SUCCESS;

    if ( lcn_priority_queue_size( term_enum->queue ) == 0 )
    {
        return LCN_ERR_ITERATOR_NO_NEXT;
    }

    do
    {
        lcn_segment_merge_info_t *top;
        unsigned int text_len;

        if ( NULL == (top = (lcn_segment_merge_info_t*) lcn_priority_queue_top( term_enum->queue) ))
        {
            term_enum->term = NULL;
            s = LCN_ERR_ITERATOR_NO_NEXT;
            break;
        }

        text_len = strlen( top->term->text );

        if ( term_enum->buffer_size < text_len )
        {
            LCNCE( lcn_term_enum_grow_buffer( term_enum, text_len + 1 ) );
        }

        apr_cpystrn( term_enum->term->text, top->term->text, text_len + 1 );
        term_enum->term->field = top->term->field;

        term_enum->doc_freq = 0;

        while( NULL != top &&
               lcn_term_compare( term_enum->term, top->term ) == 0 )
        {
            apr_status_t next_status;

            lcn_priority_queue_pop( term_enum->queue );

            lcn_term_enum_doc_freq( top->term_enum );

            term_enum->doc_freq += lcn_term_enum_doc_freq( top->term_enum );  /* increment freq */

            next_status = lcn_segment_merge_info_next( top );

            if ( APR_SUCCESS == next_status )
            {
                lcn_priority_queue_put( term_enum->queue, top );
            }
            else if ( LCN_ERR_ITERATOR_NO_NEXT == next_status ||
                      LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == next_status )
            {
                LCNCE( lcn_segment_merge_info_close( top ) );
            }
            else
            {
                s = next_status;
                break;
            }

            top = (lcn_segment_merge_info_t*) lcn_priority_queue_top(term_enum->queue);
        }
    }
    while(0);

    return s;
}

static apr_status_t
lcn_multi_term_enum_close( lcn_term_enum_t *term_enum )
{
    return lcn_segment_merge_queue_close( term_enum->queue );
}

apr_status_t
lcn_multi_term_enum_create( lcn_term_enum_t **term_enum,
                            lcn_index_reader_t **readers,
                            unsigned int *starts,
                            unsigned int readers_count,
                            lcn_term_t *term,
                            apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;

        LCNCE( lcn_base_term_enum_create( term_enum, pool ) );

        (*term_enum)->get_doc_freq = lcn_multi_term_enum_doc_freq;
        (*term_enum)->next         = lcn_multi_term_enum_next;
        (*term_enum)->close        = lcn_multi_term_enum_close;

        (*term_enum)->sub_readers = readers;
        (*term_enum)->sub_readers_count = readers_count;

        LCNCE( lcn_segment_merge_queue_create( &((*term_enum)->queue),
                                               readers_count,
                                               pool ) );

        for ( i = 0; i < readers_count; i++ )
        {
            apr_status_t next_status;
            lcn_index_reader_t *reader = readers[i];
            lcn_term_enum_t *te;
            lcn_segment_merge_info_t *smi;

            if ( NULL != term )
            {
                apr_status_t next;
                lcn_bool_t skip;

                s = lcn_index_reader_terms_from( reader, &te, term, pool );

                if ( s == LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM )
                {
                    s = APR_SUCCESS;
                    continue;
                }

                skip = te->skip_first_next;
                next = lcn_term_enum_next( te );

                if ( LCN_ERR_ITERATOR_NO_NEXT == next )
                {
                    continue;
                }
                else
                {
                    te->skip_first_next = skip;
                }
            }
            else
            {
                LCNCE( lcn_index_reader_terms( reader, &te, pool) );
            }

            LCNCE( lcn_segment_merge_info_create( &smi, starts[i], te, reader, pool ) );

            if ( NULL == term )
            {
                next_status = lcn_segment_merge_info_next( smi );

                if ( APR_SUCCESS == next_status )
                {
                    lcn_priority_queue_put( (*term_enum)->queue, smi );
                }
                else
                {
                    LCNCE( lcn_segment_merge_info_close( smi ) );
                }
            }
            else
            {
                lcn_priority_queue_put( (*term_enum)->queue, smi );
            }
        }
    }
    while(0);

    return s;
}
