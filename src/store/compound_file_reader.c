#include "lucene.h"
#include "directory.h"
#include "compound_file_reader.h"
#include "lcn_util.h"
#include "io_context.h"

apr_status_t
lcn_compound_file_reader_create ( lcn_compound_file_reader_t **cfr,
                                  lcn_directory_t *dir,
                                  const char *cf_name,
                                  apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_file_entry_t *last_entry = NULL;
        lcn_file_entry_t *new_entry = NULL;
        lcn_bool_t is_start_entry = LCN_TRUE;
        unsigned int count = 0, i = 0;

        LCNPV( *cfr = apr_pcalloc( pool, sizeof ( lcn_compound_file_reader_t ) ), APR_ENOMEM );
        (*cfr)->cf_name = cf_name;
        (*cfr)->dir = dir;
        (*cfr)->pool = pool;
        (*cfr)->is_open = LCN_TRUE;

        LCNCE( lcn_directory_open_input( (*cfr)->dir, &((*cfr)->istream), (*cfr)->cf_name, LCN_IO_CONTEXT_READONCE, (*cfr)->pool ) );
        // read the directory and init files
        LCNCE( lcn_index_input_read_vint( (*cfr)->istream, &count ));

        for ( i = 0; i < count; i++ )
        {
            char *file_name = NULL;
            unsigned int len = 0;
            apr_int64_t offset = 0;

            LCNCE( lcn_index_input_read_long( (*cfr)->istream, &offset ) );
            LCNCE ( lcn_index_input_read_string( (*cfr)->istream, &file_name, &len, (*cfr)->pool ) );

            if( new_entry != NULL )
            {
                // set length of the previous entry
                new_entry->length = offset - new_entry->data_offset;
                is_start_entry = LCN_FALSE;
            }

            LCNPV ( new_entry = apr_pcalloc( (*cfr)->pool, sizeof ( lcn_file_entry_t ) ), APR_ENOMEM );
            new_entry->data_offset = offset;
            new_entry->file_name = file_name;

            if ( is_start_entry )
            {
                (*cfr)->list_entry_start = new_entry;
            }

            if( last_entry != NULL )
            {
                last_entry->next = new_entry;
            }

            last_entry = new_entry;
        }
        LCNCE( s );

        // set the length of the final entry
        if (new_entry != NULL)
        {
            unsigned int stream_size = lcn_index_input_size( (*cfr)->istream );
            new_entry->length =  stream_size - new_entry->data_offset;
            new_entry->next = NULL;
        }
    }
    while(0);

    if ( s )
    {
        if ( (*cfr)->istream != NULL )
        {
            LCNCR ( lcn_index_input_close( (*cfr)->istream ));
        }
    }

    return s;
}

apr_status_t
lcn_compound_file_reader_close ( lcn_compound_file_reader_t *cfr )
{
    apr_status_t s = APR_SUCCESS;
    LCNASSERTR ( cfr->is_open == LCN_TRUE, LCN_ERR_STREAM_CLOSED );

    LCNCR ( lcn_index_input_close( cfr->istream ) );
    cfr->is_open = LCN_FALSE;

    return s;
}

lcn_bool_t
lcn_compound_file_reader_is_open ( lcn_compound_file_reader_t *cfr )
{
    return cfr->is_open;
}

apr_status_t
lcn_compound_file_reader_open_input ( lcn_compound_file_reader_t *cfr,
                                      lcn_index_input_t **file_istream,
                                      const char *file_name )
{
    apr_status_t s = APR_SUCCESS;
    lcn_file_entry_t *entry;

    LCNASSERTR ( cfr->is_open == LCN_TRUE, LCN_ERR_STREAM_CLOSED );

    if ( cfr->istream == NULL )
    {
        return LCN_ERR_INVALID_ARGUMENT;
    }

    entry = cfr->list_entry_start;
    while ( entry != NULL )
    {
        if (  0 == strcmp( entry->file_name, file_name ) )
        {
            break;
        }
        entry = entry->next;
    }

    if ( entry == NULL )
    {
        printf("\n\nlcn_compound_file_reader_open_input: datei nicht gefunden: %s\n", file_name);
        return LCN_ERR_CF_SUB_FILE_NOT_FOUND;
    }

    LCNCR ( lcn_cs_input_stream_create(file_istream, cfr->istream, entry->data_offset, entry->length, cfr->pool ) );

    return s;
}

unsigned int
lcn_compound_file_reader_entries_size ( lcn_compound_file_reader_t *cfr )
{
    unsigned int size = 0;
    lcn_file_entry_t *entry = cfr->list_entry_start;
    while ( entry != NULL )
    {
        size++;
        entry = entry->next;
    }

    return size;
}

apr_status_t
lcn_compound_file_reader_entries_as_list ( lcn_compound_file_reader_t *cfr,
                                    lcn_list_t **entries,
                                    apr_pool_t *pool)
{
    apr_status_t s;
    lcn_file_entry_t *entry;
    unsigned int size = lcn_compound_file_reader_entries_size(cfr);

    do
    {
        LCNCE(lcn_list_create(entries, size, pool));

        entry = cfr->list_entry_start;
        while ( entry != NULL )
        {
            LCNCE(lcn_list_add(*entries, entry->file_name));
            entry = entry->next;
        }
        LCNCE(s);
    }
    while(0);

    return s;
}

lcn_bool_t
lcn_compound_file_reader_file_name_exists( lcn_compound_file_reader_t *cfr,
                                           const char *file_name)
{
    lcn_file_entry_t *entry = cfr->list_entry_start;
    lcn_bool_t ret_val = LCN_FALSE;

    while ( entry != NULL )
    {
        if ( 0 == strcmp( entry->file_name, file_name ) )
        {
            ret_val = LCN_TRUE;
        }
        entry = entry->next;
    }

    return ret_val;
}
