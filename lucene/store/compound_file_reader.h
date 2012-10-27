#ifndef COMPOUND_FILE_READER_H
#define	COMPOUND_FILE_READER_H

#include "lucene.h"
#include "lcn_store.h"
#include "directory.h"

struct lcn_compound_file_reader_t {

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
     * Start element of the linkedlist whitch conaints all single files from the
     * .cfs File.
     */
    lcn_file_entry_t *list_entry_start;
    
    /**
     * Inputstream to the .cfs File.
     */
    lcn_istream_t *istream;
    
    /**
     * Define whether the stream is open or closed.  
     */
    lcn_bool_t is_open;
    
};

apr_status_t
lcn_compound_file_reader_create ( lcn_compound_file_reader_t **cfr, 
                                  lcn_directory_t *dir,
                                  const char *cf_name, 
                                  apr_pool_t *pool );

apr_status_t
lcn_compound_file_reader_init ( lcn_compound_file_reader_t *cfr,
                                  apr_pool_t *pool );

apr_status_t
lcn_compound_file_reader_close ( lcn_compound_file_reader_t *cfr );

apr_status_t
lcn_compound_file_reader_open_input ( lcn_compound_file_reader_t *cfr,
                                      lcn_istream_t **file_istream,
                                      const char *file_name );

unsigned int
lcn_compound_file_reader_entries_size ( lcn_compound_file_reader_t *cfr );

lcn_bool_t
lcn_compound_file_reader_is_open ( lcn_compound_file_reader_t *cfr );

lcn_bool_t
lcn_compound_file_reader_file_name_exists( lcn_compound_file_reader_t *cfr, 
                                           const char *file_name);

apr_status_t
lcn_compound_file_reader_entries_as_list ( lcn_compound_file_reader_t *cfr,
                                    lcn_list_t **entries,
                                    apr_pool_t *pool);

#endif	/* COMPOUND_FILE_READER_H */
