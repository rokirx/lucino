#ifndef COMPOUND_FILE_WRITER_H
#define	COMPOUND_FILE_WRITER_H

#include "lucene.h"
#include "lcn_store.h"

struct lcn_compound_file_writer_t {

    apr_pool_t *pool;

    /**
     * Path to locate index files.
     */
    lcn_directory_t *dir;
    
    /**
     * Name of the basic .cfs File.
     */
    const char *cf_name;
    
    /**
     * Index whitch will combined in one .cfs-File
     */
    lcn_list_t *entries;
    
    /**
     * List which contains all names of seq_files.
     */
    lcn_list_t *ids;
    
    lcn_bool_t is_open; 
    
    lcn_bool_t merged;

};

apr_status_t
lcn_compound_file_writer_create ( lcn_compound_file_writer_t **cfw, 
                                  lcn_directory_t *dir,
                                  const char *cf_name, 
                                  apr_pool_t *pool );
apr_status_t
lcn_compound_file_writer_add_file( lcn_compound_file_writer_t *cfw,
                                   const char *seq_file );

unsigned int
lcn_compound_file_writer_entries_size( lcn_compound_file_writer_t *cfw );

unsigned int
lcn_compound_file_writer_ids_size( lcn_compound_file_writer_t *cfw );

apr_status_t
lcn_compound_file_writer_close( lcn_compound_file_writer_t *cfw );

#endif	/* COMPOUND_FILE_WRITER_H */

