#include "lucene.h"
#include "lcn_index.h"

#include "document.h"
#include "fields_reader.h"
#include "directory.h"
#include "field.h"

/**
 * uncompress is not implemented. Use any compression library and store
 * compressed data as binary field.
 */

apr_status_t
lcn_fields_reader_create( lcn_fields_reader_t **fields_reader,
                          lcn_directory_t *directory,
                          const char *segment,
                          lcn_field_infos_t *field_infos,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *fields_reader = (lcn_fields_reader_t*) apr_pcalloc( pool, sizeof(lcn_fields_reader_t) ), APR_ENOMEM );

        (*fields_reader)->field_infos = field_infos;

        LCNCE( lcn_directory_open_segment_file( directory,
                                                &((*fields_reader)->fields_stream),
                                                segment,
                                                ".fdt",
                                                pool ) );


        LCNCE( lcn_directory_open_segment_file( directory,
                                                &((*fields_reader)->index_stream),
                                                segment,
                                                ".fdx",
                                                pool ) );
        LCNCE( lcn_directory_segments_format( directory, &( (*fields_reader)->format )));

        (*fields_reader)->size = (unsigned int) (lcn_index_input_size( (*fields_reader)->index_stream ) / 8);
    }
    while(0);

    return s;
}

apr_status_t
lcn_fields_reader_close( lcn_fields_reader_t *fields_reader )
{
    apr_status_t s1 = lcn_index_input_close( fields_reader->fields_stream );
    apr_status_t s2 = lcn_index_input_close( fields_reader->index_stream );

    if ( s1 )
    {
        return s1;
    }

    return s2;
}

unsigned int
lcn_fields_reader_size( lcn_fields_reader_t *fields_reader )
{
    return fields_reader->size;
}

apr_status_t
lcn_fields_reader_doc( lcn_fields_reader_t *fields_reader,
                       lcn_document_t **document,
                       unsigned int n,
                       apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        apr_int64_t position;
        unsigned int i, num_fields;
        unsigned int ui_num_fields;

        lcn_index_input_t *f_in = fields_reader->fields_stream;
        lcn_index_input_t *i_in = fields_reader->index_stream;

        LCNCE( lcn_index_input_seek( i_in, ((apr_off_t) n) * 8L ) );
        LCNCE( lcn_index_input_read_long( i_in, &position ) );
        LCNCE( lcn_index_input_seek( f_in, (apr_off_t) position ));
        LCNCE( lcn_document_create( document, pool ));
        LCNCE( lcn_index_input_read_vint( f_in, &ui_num_fields ) );

        num_fields = ui_num_fields;

        for ( i = 0; i < num_fields; i++)
        {
            unsigned int field_number;
            lcn_field_info_t *f_info;
            unsigned char bits;
            lcn_field_t *field;
            lcn_field_type_t field_type = {0};

            LCNCE( lcn_index_input_read_vint( f_in, &field_number ) );
            LCNCE( lcn_field_infos_by_number( fields_reader->field_infos, &f_info, field_number ) );
            LCNCE( lcn_index_input_read_byte( f_in, &bits ) );

            if ( 0 == fields_reader->format )
            {
                if ( bits & 0x4 ) /* LUCENE_FIELD_BINARY */
                {
                    (void) lcn_field_type_set_binary( &field_type, LCN_TRUE );
                }

                if ( bits & 0x1 ) /* LUCENE_FIELD_TOKENIZED */
                {
                    (void) lcn_field_type_set_tokenized( &field_type, LCN_TRUE );
                }
            }
            else
            {
                if ( bits & LCN_FIELD_BINARY )
                {
                    (void) lcn_field_type_set_binary( &field_type, LCN_TRUE );
                }

                if ( ( 0 != (LCN_FIELD_TOKENIZED & bits ) ) )
                {
                    lcn_field_type_set_tokenized( &field_type, LCN_TRUE );
                }
            }

            if ( lcn_field_type_is_binary( &field_type ) )
            {
                unsigned int binary_len;
                char *buf;

                LCNCE( lcn_index_input_read_vint( f_in, &binary_len ) );
                LCNPV( buf = (char*) apr_palloc( pool, sizeof(char) * binary_len ), APR_ENOMEM );
                LCNCE( lcn_index_input_read_bytes( f_in, buf, 0, &binary_len ) );
                LCNCE( lcn_field_create_binary( &field,
                                                f_info->name,
                                                buf,
                                                binary_len,
                                                pool ));
            }
            else
            {
                unsigned int len;
                char *buf = NULL;

                LCNCE( lcn_field_type_set_stored( &field_type, LCN_TRUE ));

                if ( lcn_field_info_fixed_size( f_info ) )
                {
                    lcn_field_type_set_fixed_size( &field_type, LCN_TRUE );
                }

                if ( lcn_field_info_is_indexed( f_info ) )
                {
                    LCNCE( lcn_field_type_set_indexed( &field_type, LCN_TRUE ));
                }

                if ( lcn_field_info_omit_norms( f_info ) )
                {
                    LCNCE( lcn_field_type_set_omit_norms( &field_type, LCN_TRUE ));
                }

                if ( lcn_field_info_store_term_vector( f_info ) )
                {
                    LCNCE( lcn_field_type_set_store_term_vectors( &field_type, LCN_TRUE ));
                }

                if ( lcn_field_info_store_position_with_term_vector( f_info ) )
                {
                    LCNCE( lcn_field_type_set_store_term_vector_positions( &field_type, LCN_TRUE ));
                }

                if ( lcn_field_info_store_offset_with_term_vector( f_info ) )
                {
                    LCNCE( lcn_field_type_set_store_term_vector_offsets( &field_type, LCN_TRUE ));
                }

                LCNCE( lcn_index_input_read_string( f_in, &buf, &len, pool ) );

                LCNCE( lcn_field_create( &field, f_info->name, buf, &field_type, pool ));
            }

            LCNCE( lcn_document_add_field( *document, field ));
        }

        (*document)->index_pos = n;
    }
    while(0);

    return s;
}
