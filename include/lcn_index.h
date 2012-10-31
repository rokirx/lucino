#ifndef LCN_INDEX_H
#define LCN_INDEX_H

#include "lucene.h"
#include "lcn_util.h"

/**
 * @defgroup lcn_index Index
 * @ingroup lucene
 * @{
 */

typedef struct lcn_norm_t lcn_norm_t;
typedef struct lcn_term_docs_t lcn_term_docs_t;
typedef struct lcn_fs_field_t lcn_fs_field_t;

/**
 * @defgroup field_type FieldType
 * @ingroup lcn_index
 * @{
 */

/**
 * @deprecated
 * @brief  Used for tokenized fields
 *
 * Only indexed fields can be tokenized using an analyzer.
 * This is used for full text search for those fields.
 */
#define LCN_FIELD_TOKENIZED               (   0x1)

#define LCN_FIELD_BINARY                  (   0x2)
#define LCN_FIELD_COMPRESSED              (   0x4)
#define LCN_FIELD_INDEXED                 (   0x8)
#define LCN_FIELD_OMIT_NORMS              (  0x10)
#define LCN_FIELD_STORED                  (  0x20)
#define LCN_FIELD_STORE_TERM_VECTOR       (  0x40)
#define LCN_FIELD_STORE_OFFSET_WITH_TV    (  0x80)
#define LCN_FIELD_STORE_POSITION_WITH_TV  ( 0x100)


/**
 * @brief  Field Types
 *
 * Only indexed fields can be tokenized using an analyzer.
 * This is used for full text search for those fields.
 */

#define LCN_FIELD_TYPE_BINARY                      (   0x1)
#define LCN_FIELD_TYPE_COPY_VALUE                  (   0x2)
#define LCN_FIELD_TYPE_FIXED_SIZE                  (   0x4)
#define LCN_FIELD_TYPE_FROSEN                      (   0x8)
#define LCN_FIELD_TYPE_INDEXED                     (  0x10)
#define LCN_FIELD_TYPE_OMIT_NORMS                  (  0x20)
#define LCN_FIELD_TYPE_STORED                      (  0x40)
#define LCN_FIELD_TYPE_STORE_TERM_VECTORS          (  0x80)
#define LCN_FIELD_TYPE_STORE_TERM_VECTOR_OFFSETS   ( 0x100)
#define LCN_FIELD_TYPE_STORE_TERM_VECTOR_POSITIONS ( 0x200)
#define LCN_FIELD_TYPE_TOKENIZED                   ( 0x400)




/**
 * @brief Used for fields of fixed size.
 *
 * The fields of fixed size are stored in separate
 * segment filed and need not to be merged. The field contents
 * can be accessed using calculated offset information. They
 * can be also loaded into memory at once for very fast access
 * to field contents
 */
#define LCN_FIELD_FIXED_SIZE              ( 0x200)

#define LCN_FIELD_VALUE_COPY    (0)
#define LCN_FIELD_NO_VALUE_COPY (1)


/** @} */

BEGIN_C_DECLS

/**
 * @defgroup lcn_term_docs TermDocs
 * @ingroup lcn_index
 * @{
 */

apr_status_t
lcn_term_docs_seek_term( lcn_term_docs_t *term_docs,
                         const lcn_term_t *term );

apr_status_t
lcn_term_docs_seek_term_enum( lcn_term_docs_t *term_docs,
                              lcn_term_enum_t *term_enum );

apr_status_t
lcn_term_docs_close( lcn_term_docs_t *term_docs );

unsigned int
lcn_term_docs_doc( lcn_term_docs_t *term_docs );

unsigned int
lcn_term_docs_freq( lcn_term_docs_t *term_docs );

apr_status_t
lcn_term_docs_next( lcn_term_docs_t *term_docs );

apr_status_t
lcn_term_positions_next_position( lcn_term_docs_t *term_docs,
                                  apr_ssize_t *position );

void test( lcn_int_array_t* test_arr );

apr_status_t
lcn_term_docs_read( lcn_term_docs_t *term_docs,
                    lcn_int_array_t* docs,
                    lcn_int_array_t* freqs,
                    unsigned int *read_entries );

apr_status_t
lcn_term_docs_skip_to( lcn_term_docs_t *term_docs,
                       unsigned int target );


/** @} */

/**
 * @defgroup lcn_index_fs_field Fixed Size Field
 * @ingroup lcn_index
 * @{
 */

/**
 * @brief Retrieves the field value of the document given by doc_id
 *
 * IMPORTANT: The size of val buffer must be big enough to hold the field data
 *
 * @param field    The field object
 * @param val      Stores the field value of the document
 * @param doc_id   The internal doc id of the document
 */
apr_status_t
lcn_fs_field_value( lcn_fs_field_t *field,
                    char *val,
                    unsigned int doc_id );

/**
 * @brief Merges the fields of many indexes
 *
 * @param dir       Directory, where the indexes will be merged
 * @param dir_list  Directories with the indexes to add
 */
apr_status_t
lcn_fs_field_merge_indexes( lcn_directory_t *dir,
                            lcn_list_t *dir_list,
                            apr_pool_t *pool );

/**
 * @brief Returns the name of the field
 *
 * @param field  The field object
 */
const char *
lcn_fs_field_name( lcn_fs_field_t *field );

/**
 * @brief Returns the number of documents in the field
 *
 * @param field  The field object
 */
unsigned int
lcn_fs_field_docs_count( lcn_fs_field_t *field );

/**
 * @brief Returns the data size of the field data in bits
 *
 * @param field  The field object
 */
unsigned int
lcn_fs_field_data_size( lcn_fs_field_t *field );



/**
 * @brief Retrieves the field value of the document as int
 *
 * @param field    The field object
 * @param val      Integer value of the field
 * @param doc_id   The internal doc id of the document
 */
apr_status_t
lcn_fs_field_int_value( lcn_fs_field_t *field,
                        unsigned int *val,
                        unsigned int doc_id );

/**
 * @brief Sets a new default value.
 *
 * @param field          The field object
 * @param default_value  new default value
 */
apr_status_t
lcn_fs_field_set_default_int_value( lcn_fs_field_t *field,
                                    unsigned int default_value );

/**
 * @brief Writes a possibly changed or new fixed size
 *        field to the field directory. It also updates
 *        the definition file of the fixed size fields
 *        in the directory
 *
 * @param field
 * @param pool
 */
apr_status_t
lcn_fs_field_commit( lcn_fs_field_t *field,
                     apr_pool_t *pool );

/**
 * @brief Writes a possibly changed or new fixed size
 *        field to the field directory and closes it. It also updates
 *        the definition file of the fixed size fields in the directory.
 *
 * @param field
 * @param pool
 */
apr_status_t
lcn_fs_field_close( lcn_fs_field_t *field,
                    apr_pool_t *pool );


/**
 * @brief Closes all fields in the hash (char* -> lcn_fs_field_t*)
 *
 * @param fields  Hash with fields: (char* -> lcn_fs_field_t*)
 */
apr_status_t
lcn_fs_field_close_fields_in_hash( apr_hash_t *fields );


/**
 * @brief Converts an fs_field to field_info to retrieve
 *        infos about field via index_reader
 *
 * @param fs_field    The field object
 * @param field_info  New field_info object
 * @param pool        APR-pool
 */
apr_status_t
lcn_fs_field_to_field_info( lcn_fs_field_t* fs_field,
                            lcn_field_info_t **field_info,
                            apr_pool_t *pool );

#define LCN_FIELD_TYPE_SETTER_DECL( NAME )              \
apr_status_t                                            \
lcn_field_type_set_##NAME( lcn_field_type_t *ft,        \
                           lcn_bool_t val );            \
                                                        \
lcn_bool_t                                              \
lcn_field_type_is_##NAME( lcn_field_type_t ft );


/**
 * Setter und getter for field_type
 */
LCN_FIELD_TYPE_SETTER_DECL( binary                      )
LCN_FIELD_TYPE_SETTER_DECL( copy_value                  )
LCN_FIELD_TYPE_SETTER_DECL( fixed_size                  )
LCN_FIELD_TYPE_SETTER_DECL( indexed                     )
LCN_FIELD_TYPE_SETTER_DECL( omit_norms                  )
LCN_FIELD_TYPE_SETTER_DECL( stored                      )
LCN_FIELD_TYPE_SETTER_DECL( store_term_vectors          )
LCN_FIELD_TYPE_SETTER_DECL( store_term_vector_offsets   )
LCN_FIELD_TYPE_SETTER_DECL( store_term_vector_positions )
LCN_FIELD_TYPE_SETTER_DECL( tokenized                   )


apr_status_t
lcn_field_type_init( lcn_field_type_t *ft );

#if 0
/**
 * @brief Updates the fields definitions in the
 *        description file 'fsf'.
 *
 * @param field
 * @param dir
 * @param pool
 */
apr_status_t
lcn_fs_field_update_fields_def( lcn_directory_fs_field_t *field,
                                lcn_directory_t *dir,
                                apr_pool_t *pool );

#endif

/*@}*/

/**
 * @defgroup lcn_index_reader Index-Reader
 * @ingroup lcn_index
 * @{
 */

typedef struct lcn_index_reader_t lcn_index_reader_t;

/**
 * @brief Fills integer values of a field into an array.
 *
 * The size of array is max_doc of the index. The result is cached
 * in the index reader.
 *
 * The memory for the array is allocated from the index_reader pool.
 *
 * @param reader       Index reader used to retrieve field values
 * @param int_array    Double pointer to store the resulting array
 * @param field_name   Field name to fetch values for
 * @param default_val  Default int value to stor in the array for
 *                     documents not having the field
 */
apr_status_t
lcn_index_reader_get_int_field_values( lcn_index_reader_t *reader,
                                       lcn_int_array_t **int_array,
                                       const char *field_name,
                                       int default_val );

apr_status_t
lcn_index_reader_create_by_path( lcn_index_reader_t **index_reader,
                                 const char *path,
                                 apr_pool_t *pool );

apr_status_t
lcn_index_reader_create_by_directory( lcn_index_reader_t **index_reader,
                                      lcn_directory_t* dir,
                                      apr_pool_t *pool );

/**
 * @brief Creates a multi reader out of list of readers
 *
 * @param reader       Index reader object to create
 * @param sub_readers  List of readers
 * @param pool         APR Pool
 */
apr_status_t
lcn_multi_reader_create_by_sub_readers( lcn_index_reader_t **reader,
                                        lcn_list_t *sub_readers,
                                        apr_pool_t *pool );

/**
 * @brief Adds a new fixed size field to already existing reader
 *
 * @param index_reader  Index reader object
 * @param field         Field to add
 */
apr_status_t
lcn_index_reader_add_fs_field_def( lcn_index_reader_t *index_reader,
                                   lcn_field_t * field );

/**
 * @brief Sets the new int value of the fixed size field named 'field_name'
 *        for the document 'docid'
 *
 * @param reader      Index reader object
 * @param docid       Internal docid of the document
 * @param field_name  Field name of the fixed sized field
 * @param int_value   New integer value of the field
 */
apr_status_t
lcn_index_reader_set_int_value( lcn_index_reader_t *reader,
                                unsigned int docid,
                                const char *field_name,
                                unsigned int int_value );

/**
 * @brief  Creates a bitvector based on some fixed size field
 *
 * For each document of the index the value of field fname is
 * evaluated using the filter_function. If the filter_function
 * returns LCN_TRUE for a document, then the corresponding bit
 * of the bitvector for this document is set to 1. For documents
 * not having the field fname the corresponding bit is set to 0.
 * Those documents can occur while using multi indexes.
 *
 * @param reader           Underlying index reader
 * @param bitvector        Stores the new bitvector
 * @param fname            Name of the field
 * @param filter_function  Filter function used to evaluate the field
 * @param pool             APR pool
 */
apr_status_t
lcn_index_reader_fs_field_bitvector( lcn_index_reader_t *reader,
                                     lcn_bitvector_t **bitvector,
                                     const char *fname,
                                     lcn_bool_t (*filter_function)( lcn_fs_field_t *, unsigned int ),
                                     apr_pool_t *pool );

/**
 * @brief Get fixed sized field
 *
 * @param reader       Underlying index reader
 * @param field        Resulting field
 * @param field_name   Name of the field
 */
apr_status_t
lcn_index_reader_get_fs_field( lcn_index_reader_t *reader,
                               lcn_fs_field_t **field,
                               const char *field_name );

/**
 * @brief Initialize a boolean to indicate whether a field named
 *        field_name exists.
 *
 * @param index_reader Underlying index reader
 * @param has_field    Stores the boolean value. True, if the field exists
 * @param field_name   Name of the field
 */
apr_status_t
lcn_index_reader_has_field( lcn_index_reader_t *index_reader,
                            lcn_bool_t *has_field,
                            const char *field_name );

apr_status_t
lcn_index_reader_fs_int_field_bitvector( lcn_index_reader_t *reader,
                                         lcn_bitvector_t **bitvector,
                                         const char *fname,
                                         lcn_bool_t (*filter_function) (void* data, unsigned int val, unsigned int doc_order ),
                                         void *filter_data,
                                         apr_pool_t *pool );

void
lcn_index_reader_set_log_stream( lcn_index_reader_t *index_reader,
                                 FILE *log_stream );

apr_status_t
lcn_index_reader_norms( lcn_index_reader_t* index_reader,
                        lcn_byte_array_t** norms,
                        const char* field );

apr_status_t
lcn_index_reader_read_norms_to_array( lcn_index_reader_t* index_reader,
                                      lcn_byte_array_t* norms,
                                      unsigned int offset,
                                      const char* field );

/**
 * @brief Number of documents in the index, not including
 *        deleted documents.
 *
 * @param index_reader
 */
unsigned int
lcn_index_reader_num_docs( lcn_index_reader_t *index_reader );

/**
 * @brief Max doc number in the index.
 *
 * @param index_reader
 */
unsigned int
lcn_index_reader_max_doc( lcn_index_reader_t *index_reader );

apr_status_t
lcn_index_reader_null_bitvector( lcn_index_reader_t *reader,
                                 lcn_bitvector_t **bitvector,
                                 apr_pool_t *pool );

lcn_bool_t
lcn_index_reader_has_deletions( lcn_index_reader_t *index_reader );

apr_status_t
lcn_index_reader_delete( lcn_index_reader_t *index_reader, int doc_num );


/**
 * @brief  Deletes all documents containing term.
 *
 * This is useful if one uses a document field to hold a unique ID string for
 * the document.  Then to delete such a document, one merely constructs a
 * term with the appropriate field and the unique ID string as its text and
 * passes it to this method.
 */
apr_status_t
lcn_index_reader_delete_documents( lcn_index_reader_t *index_reader,
                                   lcn_term_t *term,
                                   unsigned int *docs_deleted );


apr_status_t
lcn_index_reader_undelete_all( lcn_index_reader_t *index_reader );

apr_status_t
lcn_index_reader_commit( lcn_index_reader_t *index_reader );

apr_status_t
lcn_index_reader_close( lcn_index_reader_t *index_reader );

apr_status_t
lcn_index_reader_terms( lcn_index_reader_t *index_reader,
                        lcn_term_enum_t **term_enum,
                        apr_pool_t *pool );

apr_status_t
lcn_index_reader_terms_from( lcn_index_reader_t *index_reader,
                             lcn_term_enum_t **term_enum,
                             lcn_term_t *term,
                             apr_pool_t *pool );
/**
 * @brief Get a document by id
 *
 * @param index_reader   Index reader to retrieve the document
 * @param document       Stores the retrieved document
 * @param n              Internal document id
 * @param pool           APR pool
 */
apr_status_t
lcn_index_reader_document( lcn_index_reader_t *index_reader,
                           lcn_document_t **document,
                           unsigned int n,
                           apr_pool_t *pool );

lcn_bool_t
lcn_index_reader_is_deleted( lcn_index_reader_t *index_reader,
                             unsigned int n );

apr_status_t
lcn_index_reader_term_docs( lcn_index_reader_t *index_reader,
                            lcn_term_docs_t **term_docs,
                            apr_pool_t *pool );

apr_status_t
lcn_index_reader_term_docs_from( lcn_index_reader_t *index_reader,
                                 lcn_term_docs_t **term_docs,
                                 const lcn_term_t *term,
                                 apr_pool_t *pool );

apr_status_t
lcn_index_reader_term_positions( lcn_index_reader_t *index_reader,
                                 lcn_term_docs_t **term_positions,
                                 apr_pool_t *pool );

apr_status_t
lcn_index_reader_term_positions_by_term( lcn_index_reader_t *index_reader,
                                         lcn_term_docs_t **term_positions,
                                         lcn_term_t *term,
                                         apr_pool_t *pool );

apr_status_t
lcn_index_reader_doc_freq( lcn_index_reader_t *index_reader,
                           const lcn_term_t *term,
                           int *freq );

lcn_bool_t
lcn_index_reader_has_norms( lcn_index_reader_t *index_reader,
                            const char *field );

/**
 * @brief Returns the directory this index resides in.
 */
lcn_directory_t*
lcn_index_reader_directory( lcn_index_reader_t *index_reader );

/**
 * @brief  Completes the field_infos list with the specified field infos
 *
 * All field infos of the index reader are checked to have the same
 * field properties as in field_flags, only those properties masked
 * by options_mask are relevant for this check. If a field info
 * satisfies this conditions and is not already an element of field_infos
 * then it is added to field_infos list
 *
 * The fixed sized fields do not have corresponding field infos and therefore
 * do not occur in any field infos lists.
 *
 * @param index_reader  index reader to get field infos of
 * @param field_infos   A list with field infos (#lcn_field_info_t)
 * @param field_flags   the must have value of properties
 * @param options_mask  Bit mask of options wich must be compared with the field_flags
 */
apr_status_t
lcn_index_reader_get_field_infos( lcn_index_reader_t *index_reader,
                                  lcn_list_t *field_infos,
                                  unsigned int field_flags,
                                  unsigned int options_mask );

apr_status_t
lcn_segment_reader_files( lcn_index_reader_t *segment_reader,
                          lcn_list_t **files,
                          apr_pool_t *pool );


/** @} */

/**
 * @defgroup lcn_index_writer Index-Writer
 * @ingroup lcn_index
 * @{
 */

typedef struct lcn_index_writer_t lcn_index_writer_t;

/**
 * @brief Creates a lucene index from a dump
 *
 * @param index_path   Path where the new index will be created
 * @param dump_file    File name of the dump file
 * @param analyzer_map Hash map analyzer name to analyzer
 * @param optimize     Whether to optimize the index
 * @param pool         Apr pool
 */
apr_status_t
lcn_index_writer_create_index_by_dump( const char* index_path,
                                       const char* dump_file,
                                       apr_hash_t* analyzer_map,
                                       lcn_bool_t  optimize,
                                       apr_pool_t *pool );

apr_status_t
lcn_index_writer_create_by_path( lcn_index_writer_t **index_writer,
                                 const char *path,
                                 lcn_bool_t create,
                                 apr_pool_t *pool );

apr_status_t
lcn_index_writer_create_by_directory( lcn_index_writer_t **index_writer,
                                      lcn_directory_t *directory,
                                      lcn_bool_t create,
                                      apr_pool_t *pool );

apr_status_t
lcn_index_writer_create_by_config( lcn_index_writer_t **index_writer, 
                                   lcn_directory_t *directory,
                                   lcn_index_writer_config_t *config,
                                   apr_pool_t *pool );

void
lcn_index_writer_set_log_stream( lcn_index_writer_t *index_writer,
                                 FILE *log_stream );

void
lcn_index_writer_set_max_buffered_docs( lcn_index_writer_t *index_writer,
                                        unsigned int max_buffered_docs );

void
lcn_index_writer_set_merge_factor( lcn_index_writer_t *index_writer,
                                   unsigned int merge_factor );

lcn_similarity_t *
lcn_index_writer_get_similarity( lcn_index_writer_t *index_writer );

unsigned int
lcn_index_writer_get_max_field_length( lcn_index_writer_t *index_writer );

void
lcn_index_writer_set_term_index_interval( lcn_index_writer_t *index_writer,
                                          unsigned int term_index_interval );

unsigned int
lcn_index_writer_get_term_index_interval( lcn_index_writer_t *index_writer );

void
lcn_index_writer_set_config( lcn_index_writer_t *index_writer,
                             lcn_index_writer_config_t *config );

/**
 * @brief Returns total number of docs in this index, including
 *  docs not yet flushed (still in the RAM buffer),
 *  not counting deletions.
 *
 * @param index_writer
 */
unsigned int
lcn_index_writer_max_doc( lcn_index_writer_t *index_writer );


/**
 * @brief  Adds a lucene document to index
 *
 * All fields of the document are handled (analyzed, indexed or stored)
 * and written to indexed.
 *
 * @param index_writer  The index writer to write to
 * @param document      Document to be written
 */
apr_status_t
lcn_index_writer_add_document( lcn_index_writer_t *index_writer,
                               lcn_document_t *document );

apr_status_t
lcn_index_writer_close( lcn_index_writer_t *index_writer );

/**
 * @brief  Optimizes the index writer
 *
 * This function must be called _after_ closing the index writer
 *
 * @param index_writer  Index writer for optimization
 */
apr_status_t
lcn_index_writer_optimize( lcn_index_writer_t *index_writer );

/**
 * @brief Merges all segments from an array of indexes into this index.
 *
 * This may be used to parallelize batch indexing.  A large document
 * collection can be broken into sub-collections.  Each sub-collection can be
 * indexed in parallel, on a different thread, process or machine.  The
 * complete index can then be created by merging sub-collection indexes
 * with this method.
 *
 * After this completes, the index is optimized.
 */
apr_status_t
lcn_index_writer_add_indexes( lcn_index_writer_t *index_writer,
                              lcn_list_t *dirs );

apr_status_t
lcn_index_writer_delete_if_empty( lcn_index_writer_t *index_writer );


/** @} */


typedef struct lcn_segment_merge_info_t lcn_segment_merge_info_t;

apr_status_t
lcn_segment_merge_info_create( lcn_segment_merge_info_t **segment_merge_info,
                               int b,
                               lcn_term_enum_t *term_enum,
                               lcn_index_reader_t *index_reader,
                               apr_pool_t *pool );

apr_status_t
lcn_segment_merge_info_close( lcn_segment_merge_info_t *segment_merge_info );

apr_status_t
lcn_segment_merge_info_next( lcn_segment_merge_info_t *smi );

apr_status_t
lcn_segment_merge_info_get_positions( lcn_segment_merge_info_t *smi,
                                      lcn_term_docs_t **postings );

apr_status_t
lcn_segment_merge_info_get_doc_map( lcn_segment_merge_info_t *smi,
                                    lcn_int_array_t **doc_map );



/** @} */

apr_status_t
lcn_multiple_term_positions_create( lcn_term_docs_t** mtp,
                                    lcn_index_reader_t* reader,
                                    lcn_list_t* terms,
                                    apr_pool_t* pool );
apr_status_t
lcn_index_writer_cf_optimize( lcn_index_writer_t *index_writer );

/**
 * Sets exists to <code>true</code> if an index exists at the specified directory.
 * @param  directory the directory to check for an index
 * @param  exists is set <code>true</code> if an index exists; <code>false</code> otherwise
 */
apr_status_t
lcn_directory_reader_index_exists( lcn_directory_t *directory,
                                   lcn_bool_t *exists,
                                   apr_pool_t *pool );



END_C_DECLS

#endif /* LCN_INDEX_H */
