#include "lucene.h"
#include "lcn_util.h"

#include "document.h"
#include "fields_writer.h"
#include "directory.h"
#include "field.h"

/**
 * compress not implemented. Use any compression library and store
 * compressed data as binary field.
 */

struct fixed_size_ostream
{
    lcn_index_output_t *ostream;
    unsigned int doc_count;
};

apr_status_t
lcn_fields_writer_create( lcn_fields_writer_t **fields_writer,
                          lcn_directory_t *directory,
                          const char *segment,
                          lcn_field_infos_t *f_infos,
                          apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *fields_writer = (lcn_fields_writer_t*) apr_pcalloc( pool, sizeof(lcn_fields_writer_t)),
               APR_ENOMEM);

        (*fields_writer)->field_infos = f_infos;

        LCNCE( lcn_directory_create_segment_file( directory,
                                                  &((*fields_writer)->fields_stream ),
                                                  segment,
                                                  ".fdt",
                                                  pool ));

        /* TODO: if second create fails we must delete the ".fdt" file */

        LCNCE( lcn_directory_create_segment_file( directory,
                                                  &((*fields_writer)->index_stream),
                                                  segment,
                                                  ".fdx",
                                                  pool ));

        (*fields_writer)->directory = directory;
        (*fields_writer)->segment   = apr_pstrdup( pool, segment );
        (*fields_writer)->pool      = pool;
    }
    while(0);

    return s;
}

apr_status_t
lcn_fields_writer_add_document( lcn_fields_writer_t *fields_writer,
                                lcn_document_t *doc )
{
    apr_status_t s;

    do
    {
        unsigned int stored_count = 0;
        unsigned int i;
        lcn_field_t *doc_field;

        lcn_index_output_t *f_out = fields_writer->fields_stream;
        lcn_index_output_t *i_out = fields_writer->index_stream;

        LCNCE( lcn_ostream_write_long( i_out, lcn_ostream_get_file_pointer( f_out ) ));

        for( i = 0; i < lcn_list_size( doc->field_list ); i++ )
        {
            doc_field = lcn_list_get( doc->field_list, i );

            if ( lcn_field_is_stored( doc_field ) &&
                ! lcn_field_is_fixed_size( doc_field ) )
            {
                stored_count++;
            }
        }

        LCNCE( lcn_ostream_write_vint( f_out, stored_count ) );

        for( i = 0; i < lcn_list_size( doc->field_list ); i++ )
        {
            doc_field = lcn_list_get( doc->field_list, i );

            if ( lcn_field_is_fixed_size( doc_field ) )
            {
                continue;
            }

            if ( lcn_field_is_stored( doc_field ))
            {
                unsigned int number = 0;
                unsigned char byte = 0;

                LCNCE( lcn_field_infos_field_number( fields_writer->field_infos,
                                                     &number,
                                                     lcn_field_name( doc_field ) ) );

                LCNCE( lcn_ostream_write_vint( f_out, number ) );

                if ( lcn_field_is_tokenized( doc_field ) )
                {
                    byte |= LCN_FIELDS_WRITER_TOKENIZED;
                }

                if ( lcn_field_is_binary( doc_field ) )
                {
                    byte |= LCN_FIELDS_WRITER_BINARY;
                }

                LCNCE( lcn_ostream_write_byte( f_out, byte ) );

                if ( lcn_field_is_binary( doc_field ) )
                {
                    LCNCE( lcn_ostream_write_vint( f_out, lcn_field_size( doc_field ) ) );
                    LCNCE( lcn_ostream_write_bytes( f_out,
                                                    (char*)lcn_field_value( doc_field ),
                                                    lcn_field_size( doc_field ) ) );
                }
                else
                {
                    lcn_ostream_write_string( f_out, lcn_field_value( doc_field ) );
                }
            }
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_fields_writer_close( lcn_fields_writer_t *fields_writer )
{
    apr_status_t s1 = lcn_ostream_close( fields_writer->fields_stream );
    apr_status_t s2 = lcn_ostream_close( fields_writer->index_stream );

    if ( s1 )
    {
        return s1;
    }

    return s2;
}
