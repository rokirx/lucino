#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "lucene.h"
#include "lcn_store.h"
#include "compound_file_reader.h"

#define MAX_FILE_NAME_LENGTH (50)

/* should never be changed to remain compatible with java indexes */
#define SIZE_OF_LONG (8)

struct file_entry {

    /**
     * source file
     */
    char* file;

    /**
     * temporary holder for the start of directory entry for this file
     */
    off_t directory_offset;

    /**
     * temporary holder for the start of this file's data section
     */
    off_t data_offset;

    /**
     * For use in Compound FileReader only: the length of the file
     */
    off_t length;

    /**
     * For use in Compound FileReader only: the name of the file
     */
    char *file_name;

    struct file_entry* next;

};

struct lcn_directory_t {

    apr_pool_t *pool;

    /**
     * Flag to indicate whether segment version was initialized
     */
    lcn_bool_t segments_format_is_init;

    /**
     *
     */
    int segments_format;

    /**
     * Name of the directory. Must be copied on construction and freed
     * on deletion.
     */
    char *name;

    /**
     * Flag to mark whether directory is open
     */
    bool is_open;

    /**
     * Renames file
     */
    apr_status_t (*_rename_file) ( lcn_directory_t *directory,
                                   const char *from,
                                   const char *to);

    /**
     * Opens a segment file in the given directory ant returns a pointer to
     * the corresponding lcn_index_input_t. self is not marked as const, because
     * some implementation might need to save the name of the file in the
     * struct.
     *
     * @param seg_name : the segment name, e.g. "_123"
     * @param ext      : the file extension including the point, e.g. ".f2"
     */
    apr_status_t (*open_segment_file) ( lcn_directory_t * directory,
                                        lcn_index_input_t **new_in,
                                        const char* seg_name,
                                        const char* ext );

    apr_status_t
    (*open_input) ( lcn_directory_t *directory,
                    lcn_index_input_t **new_in,
                    const char  *file_name,
                    apr_pool_t *pool );

    apr_status_t
    (*_create_file) ( lcn_directory_t *directory,
                      lcn_ostream_t **new_os,
                      const char *file_name,
                      apr_pool_t *pool );

    apr_status_t
    (*_list) ( const lcn_directory_t *directory,
               lcn_list_t **file_list,
               apr_pool_t *pool );


    /**
     * Removes file
     */
    apr_status_t
    (*_delete_file) ( lcn_directory_t *directory,
                      const char *file_name );

    apr_status_t
    (*_file_exists) ( const lcn_directory_t *dir,
                      const char *file_name,
                      lcn_bool_t *file_exists );

    apr_status_t
    (*_remove) ( lcn_directory_t *dir );

    /**
     * Closes the store for future operations
     */
    apr_status_t (*_close) ( lcn_directory_t * );

    /* CompoundFileDirectory */

    struct lcn_compound_file_reader_t *cfr;
};

/**
 * Creates a new file in the given directory and segment with file name
 * extension 'ext'
 *
 * @dir      Directory, in which to create the file
 * @seg_name Name of segment
 * @ext      File name extension. The file name is (seg_name + ext)
 */
apr_status_t
lcn_directory_create_segment_file ( lcn_directory_t *directory,
                                    lcn_ostream_t **new_out,
                                    const char *seg_name,
                                    const char *ext,
                                    apr_pool_t *pool );

apr_status_t
lcn_base_directory_create( lcn_directory_t **new_dir,
                           apr_pool_t *pool );

apr_status_t
lcn_base_directory_init( lcn_directory_t *directory,
                         apr_pool_t *pool );


#endif /* DIRECTORY_H */
