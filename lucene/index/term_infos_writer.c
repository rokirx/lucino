#include "lucene.h"
#include "lcn_index.h"
#include "lucene_util.h"
#include "term.h"
#include "directory.h"
#include "term_infos_writer.h"

static apr_status_t
lcn_term_infos_writer_write_term ( lcn_term_infos_writer_t *term_infos_writer,
                                   const lcn_term_t *term,
                                   unsigned int field_number )
{
    apr_status_t s;

    char *last_text = term_infos_writer->last_term.text;
    unsigned int last_text_size = term_infos_writer->last_term_buf_size;
    int start = lcn_string_difference( last_text, term->text );
    int length = strlen( term->text ) - start;

    lcn_ostream_t *out = term_infos_writer->output;

    do
    {
        char *term_buf;

        /* write shared prefix length */
        LCNCE( lcn_ostream_write_vint( out, start ) );

        /* write delta length */
        LCNCE( lcn_ostream_write_vint( out, length ) );

        /* write delta chars */
        LCNCE( lcn_ostream_write_chars( out, term->text, start, length ) );

        /* write field num */
        LCNCE( lcn_ostream_write_vint( out, field_number ) );

        term_infos_writer->last_term.field = term->field;
        term_infos_writer->last_field_number = field_number;

        do
        {
            term_buf = apr_cpystrn( last_text, term->text, last_text_size );

            /* the buffer is eventually too small */

            if ( term_buf - last_text_size + 1 == last_text )
            {
                term_infos_writer->last_term_buf_size *= 2;
                term_infos_writer->last_term.text = (char*) apr_palloc( term_infos_writer->pool,
                                                                        term_infos_writer->last_term_buf_size );

                LCNPV( last_text = term_infos_writer->last_term.text, APR_ENOMEM );

                last_text_size = term_infos_writer->last_term_buf_size;
            }
            else
            {
                break;
            }
        }
        while(1);
    }
    while(0);

    return s;
}

/**
 * Called to complete TermInfos creation.
 */
apr_status_t
lcn_term_infos_writer_close ( lcn_term_infos_writer_t *term_infos_writer )
{
    apr_status_t s;
    lcn_ostream_t *out = term_infos_writer->output;

    if ( ! term_infos_writer->is_open )
    {
        return APR_SUCCESS;
    }

    do
    {
        /* write size after format */

        LCNCE( lcn_ostream_seek( out, 4 ) );
        LCNCE( lcn_ostream_write_long( out, term_infos_writer->size ) );
        LCNCE( lcn_ostream_close( out ) );

        if ( ! term_infos_writer->is_index )
        {
            LCNCE( lcn_term_infos_writer_close( term_infos_writer->other ) );
        }

        term_infos_writer->is_open = 0;
    }
    while(0);

    return s;
}

apr_status_t
lcn_term_infos_writer_add_term ( lcn_term_infos_writer_t *term_infos_writer,
                                 const lcn_term_t *term,
                                 lcn_term_info_t *term_info,
                                 unsigned int field_number )
{
    apr_status_t s;
    lcn_ostream_t *out ;

    if ( ! term_infos_writer->is_index &&
         lcn_term_compare( term, &(term_infos_writer->last_term) ) <= 0 )
    {
        LCNRM( LCN_ERR_TERM_OUT_OF_ORDER, term_infos_writer->last_term.text );
    }

    if ( term_info->freq_pointer < term_infos_writer->last_ti.freq_pointer )
    {
        return LCN_ERR_FREQ_POINTER_OUT_OF_ORDER;
    }

    if ( term_info->prox_pointer < term_infos_writer->last_ti.prox_pointer )
    {
        return LCN_ERR_PROX_POINTER_OUT_OF_ORDER;
    }

    do
    {
        /* add an index term */

        if ( ! term_infos_writer->is_index &&
             ( 0 == ( term_infos_writer->size % term_infos_writer->index_interval ) ) )
        {
            LCNCE( lcn_term_infos_writer_add_term( term_infos_writer->other,
                                                   &(term_infos_writer->last_term),
                                                   &(term_infos_writer->last_ti),
                                                   term_infos_writer->last_field_number ));
        }

        LCNCE( lcn_term_infos_writer_write_term( term_infos_writer, term, field_number ) );

        out = term_infos_writer->output;

        /* write doc freq */
        LCNCE( lcn_ostream_write_vint( out, term_info->doc_freq ) );

        /* write pointers */
        LCNCE( lcn_ostream_write_vlong( out, term_info->freq_pointer -
                                              term_infos_writer->last_ti.freq_pointer ));

        LCNCE( lcn_ostream_write_vlong( out, term_info->prox_pointer -
                                              term_infos_writer->last_ti.prox_pointer ));

        if ( term_info->doc_freq >= term_infos_writer->skip_interval )
        {
            LCNCE( lcn_ostream_write_vint( out, term_info->skip_offset ) );
        }

        if ( term_infos_writer->is_index )
        {
            /* write pointer */

            LCNCE( lcn_ostream_write_vlong( out,
                                            lcn_ostream_get_file_pointer( term_infos_writer->other->output ) -
                                            term_infos_writer->last_index_pointer ) );
            term_infos_writer->last_index_pointer = lcn_ostream_get_file_pointer( term_infos_writer->other->output );
        }

        term_infos_writer->last_ti = *term_info;
        term_infos_writer->size++;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_term_infos_writer_initialize ( lcn_term_infos_writer_t *term_infos_writer,
                                   lcn_directory_t *directory,
                                   const char *seg_name,
                                   unsigned int interval,
                                   lcn_bool_t is_index,
                                   apr_pool_t *pool )
{
    apr_status_t s;

    term_infos_writer->is_open = 1;
    term_infos_writer->last_index_pointer = 0;

    term_infos_writer->size = 0;
    term_infos_writer->is_index = is_index;

    term_infos_writer->index_interval = interval;
    term_infos_writer->skip_interval  = LCN_TERM_INFOS_SKIP_INTERVAL;

    do
    {
        term_infos_writer->last_term.field = (char*) lcn_atom_get_str( "" );
        term_infos_writer->last_term.text = (char*) apr_palloc( pool, LCN_TERM_INFO_INITIAL_BUFFER );
        LCNPV( term_infos_writer->last_term.text, APR_ENOMEM );
        *(term_infos_writer->last_term.text) = '\0';
        term_infos_writer->last_term_buf_size = LCN_TERM_INFO_INITIAL_BUFFER;
        term_infos_writer->last_field_number = -1;

        LCNCE( lcn_directory_create_segment_file( directory,
                                                  &(term_infos_writer->output),
                                                  seg_name,
                                                  is_index ? ".tii" : ".tis",
                                                  pool ) );
        /* write format */
        LCNCE( lcn_ostream_write_int( term_infos_writer->output, LCN_TERM_INFOS_FILE_FORMAT ) );

        /* leave space for size */
        LCNCE( lcn_ostream_write_long( term_infos_writer->output, 0 ) );

        /* write index interval */
        LCNCE( lcn_ostream_write_int( term_infos_writer->output, term_infos_writer->index_interval ) );

        /* write skip interval */
        LCNCE( lcn_ostream_write_int( term_infos_writer->output, LCN_TERM_INFOS_SKIP_INTERVAL ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_term_infos_writer_create( lcn_term_infos_writer_t **term_infos_writer,
                              lcn_directory_t *directory,
                              const char *seg_name,
                              unsigned int interval,
                              apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *term_infos_writer =
               (lcn_term_infos_writer_t*)
               apr_pcalloc( pool, sizeof(lcn_term_infos_writer_t) ),
               APR_ENOMEM );

        (*term_infos_writer)->pool = pool;

        LCNCE( lcn_term_infos_writer_initialize( *term_infos_writer,
                                                 directory,
                                                 seg_name,
                                                 interval,
                                                 LCN_FALSE,
                                                 pool ));
        LCNPV( (*term_infos_writer)->other =
               (lcn_term_infos_writer_t*)
               apr_pcalloc( pool, sizeof(lcn_term_infos_writer_t) ),
               APR_ENOMEM );

        LCNCE( lcn_term_infos_writer_initialize( (*term_infos_writer)->other,
                                                 directory,
                                                 seg_name,
                                                 interval,
                                                 LCN_TRUE,
                                                 pool ));

        (*term_infos_writer)->other->other = *term_infos_writer;
    }
    while(0);

    return s;
}
