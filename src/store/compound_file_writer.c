#include "lucene.h"
#include "directory.h"
#include "compound_file_writer.h"
#include "lcn_util.h"
#include "ostream.h"
#include <sys/param.h>
#include <string.h>
#include "io_context.h"

const unsigned int BUFFER_SIZE = 1024;

/** Copy the contents of the file with specified extension into the
 *  provided output stream. Use the provided buffer for moving data
 *  to reduce memory allocation.
 */
static apr_status_t
lcn_compound_file_copy( lcn_compound_file_writer_t *cfw,
                        lcn_file_entry_t *entry,
                        lcn_index_output_t *os,
                        char buffer[])
{
    apr_status_t s = APR_SUCCESS;
    lcn_index_input_t *istream = NULL;

    apr_off_t startPtr = lcn_index_output_get_file_pointer( os );

    do
    {
        unsigned int istream_length = 0;
        unsigned int istream_length_remainder = 0;
        unsigned int chunck = BUFFER_SIZE;
        unsigned int endPtr = 0;
        unsigned int diff = 0;

        LCNCE( lcn_directory_open_input(cfw->dir, &istream, entry->file, LCN_IO_CONTEXT_READONCE, cfw->pool ) );
        istream_length = lcn_index_input_size(istream);
        istream_length_remainder = istream_length;

        while( istream_length_remainder > 0) {
            unsigned int len = LCN_MIN(chunck, istream_length_remainder);
            LCNCE( lcn_index_input_read_bytes(istream, buffer, 0, &len ) );
            LCNCE( lcn_index_output_write_bytes(os, buffer, len));
            istream_length_remainder -= len;
        }
        LCNCE(s);

        // Verify that istream_length_remainder is 0
        if (istream_length_remainder != 0)
        {
            LCNLOG("Non-zero istream_length_remainder length after copying.");
            s = LCN_ERR_CF_DATA_COPY_NOT_COMPLETE;
            break;
        }

        // Verify that the output length diff is equal to original file
        endPtr = lcn_index_output_get_file_pointer( os );
        diff = endPtr - startPtr;

        if ( diff != istream_length )
        {
            LCNLOG("Difference in the output file offsets. Does not match the original file length.");
            s = LCN_ERR_CF_DATA_DIFFERENT_OFFSETS;
            break;
        }

        if ( istream != NULL )
        {
            LCNCE( lcn_index_input_close( istream ) );
        }

    }
    while(0);

    return s;
}

apr_status_t
lcn_compound_file_writer_create (lcn_compound_file_writer_t **cfw,
                                 lcn_directory_t *dir,
                                 const char *cf_name,
                                 apr_pool_t *pool)
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        LCNPV( *cfw = apr_pcalloc( pool, sizeof ( lcn_compound_file_writer_t ) ), APR_ENOMEM );
        LCNCE( lcn_list_create( &((*cfw)->entries), 4, pool ) );
        LCNCE( lcn_list_create( &((*cfw)->ids), 4, pool ) );
        (*cfw)->cf_name = cf_name;
        (*cfw)->dir = dir;
        (*cfw)->merged = LCN_FALSE;
        (*cfw)->pool = pool;
        (*cfw)->is_open = LCN_TRUE;
    }
    while(0);

    if ( s )
    {
        cfw = NULL;
    }

    return s;
}

/** Add a source stream. <code>file</code> is the string by which the
 *  sub-stream will be known in the compound stream.
 */
apr_status_t
lcn_compound_file_writer_add_file( lcn_compound_file_writer_t *cfw,
                                   const char *seq_file)
{
    apr_status_t s = APR_SUCCESS;
    int i = 0, ids_size = 0;
    lcn_file_entry_t *new_file_entry = NULL;

    LCNASSERTR ( cfw->is_open == LCN_TRUE, LCN_ERR_STREAM_CLOSED );
    ids_size = lcn_compound_file_writer_ids_size( cfw );

    bool seq_file_exists = LCN_FALSE;

    do
    {

        for ( i = 0; i < ids_size; i++ )
        {
            char *id;
            LCNPV( id = lcn_list_get( cfw->ids, i ), LCN_ERR_NULL_PTR );

            if ( 0 == strcmp( id, seq_file ) )
            {
                seq_file_exists = LCN_TRUE;
            }
        }
        LCNCE( s );

        if ( !seq_file_exists )
        {
            LCNCR ( lcn_list_add( cfw->ids, (char*)seq_file ) );

            LCNPV ( new_file_entry = apr_pcalloc( cfw->pool, sizeof ( lcn_file_entry_t ) ), APR_ENOMEM );
            new_file_entry->file = (char*)seq_file;
            LCNCR ( lcn_list_add( cfw->entries, new_file_entry ) );
        }
    }
    while(0);

    return s;
}

unsigned int
lcn_compound_file_writer_entries_size( lcn_compound_file_writer_t *cfw )
{
    return lcn_list_size( cfw->entries );
}

unsigned int
lcn_compound_file_writer_ids_size( lcn_compound_file_writer_t *cfw )
{
    return lcn_list_size( cfw->ids );
}

/** Merge files with the extensions added up to now.
 *  All files with these extensions are combined sequentially into the
 *  compound stream. After successful merge, the source files
 *  are deleted.
 *  if close() had been called before or if no file has been added to this object
 *  LCNCR -> error
 */
apr_status_t
lcn_compound_file_writer_close( lcn_compound_file_writer_t *cfw )
{
    apr_status_t s = APR_SUCCESS;
    LCNASSERTR ( cfw->is_open == LCN_TRUE, LCN_ERR_STREAM_CLOSED );
    unsigned int entries_size = 0;
    int i = 0;
    char buffer[BUFFER_SIZE];
    apr_pool_t *child_pool = NULL;

    if ( cfw->merged )
    {
        LCNLOG("Merge already performed");
        return LCN_ERR_INVALID_ARGUMENT;
    }

    entries_size = lcn_compound_file_writer_entries_size( cfw );

    if ( entries_size <= 0 )
    {
        LCNLOG("No entries to merge ...");
        return LCN_ERR_INVALID_ARGUMENT;
    }

    cfw->merged = LCN_TRUE;

    do
    {
        lcn_index_output_t *os = NULL;

        LCNCE( apr_pool_create( &child_pool, cfw->pool ));

        // open the compound stream
        LCNCE( lcn_directory_create_output( cfw->dir, &os, cfw->cf_name, child_pool ));

        // Write the number of entries
        LCNCE( lcn_index_output_write_vint( os, entries_size ) );

        /* Write the directory with all offsets at 0.
         * Remember the positions of directory entries so that we can
         * adjust the offsets later
         */
        for( i = 0; i < entries_size; i++ )
        {
            lcn_file_entry_t *entry;

            LCNPV( entry = lcn_list_get( cfw->entries, i ), LCN_ERR_NULL_PTR );
            entry->directory_offset = lcn_index_output_get_file_pointer( os );
            LCNCE( lcn_index_output_write_long( os, 0 ) ); //for now
            LCNCE( lcn_index_output_write_string( os, entry->file ) );
        }
        LCNCE ( s );

        /* Open the files and copy their data into the stream.
         * Remember the locations of each file's data section.
         */
        for( i = 0; i < entries_size; i++ )
        {
            lcn_file_entry_t *entry;

            LCNPV ( entry = lcn_list_get( cfw->entries, i ), LCN_ERR_NULL_PTR );
            entry->data_offset = lcn_index_output_get_file_pointer( os );
            LCNCE(lcn_compound_file_copy( cfw, entry, os, buffer ));
        }
        LCNCE ( s );

        // Write the data offsets into the directory of the compound stream
        for( i = 0; i < entries_size; i++ )
        {
            lcn_file_entry_t *entry;

            LCNPV ( entry = lcn_list_get( cfw->entries, i ), LCN_ERR_NULL_PTR );
            LCNCE ( lcn_index_output_seek( os, entry->directory_offset ) );
            LCNCE ( lcn_index_output_write_long( os, entry->data_offset ) );
        }
        LCNCE ( s )

        if ( os != NULL )
        {
            LCNCE ( lcn_index_output_close(os) );
        }
    }
    while(0);

    if ( child_pool != NULL )
    {
        apr_pool_destroy( child_pool );
    }

    cfw->is_open = LCN_FALSE;

    return s;
}
