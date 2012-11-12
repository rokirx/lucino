#ifndef INDEX_READER_H
#define INDEX_READER_H

#include "lucene.h"
#include "fields_reader.h"
#include "lcn_bitvector.h"
#include "term_infos_reader.h"
#include "segment_infos.h"
#include "fs_field.h"

/**
 * @defgroup lcn_index_priv Index
 * @ingroup lucene_priv
 * @{
 */


/**
 * @defgroup lcn_index_reader_priv IndexReader
 * @ingroup lucene_index_priv
 * @{
 */

struct lcn_index_reader_t {

    apr_pool_t *pool;



    /**
     * put reviewed and new lucene 4.0 members above this comment
     */

    /**
     * APR pool
     */
    apr_pool_t *norms_pool;

    /**
     * Hash containing cached field values used for sorting. In Java
     * Lucene it is in FieldCacheImpl.java.
     *
     * Field values are returned by the lcn_index_reader_get_<type>_field_values
     * functions using field_name + <data type> as a key. Example:
     * for a field "int_field" and int data type the key string is "int_field<int>".
     */
    apr_hash_t *field_values_cache;

    /**
     * Hash with the fixed size fields
     */
    apr_hash_t *fs_fields;

    /**
     * Underlying directory
     */
    lcn_directory_t *directory;

    /**
     * Segment infos of the index
     */
    lcn_segment_infos_t *segment_infos;

    /**
     * Whether the reader owns the directory, which means
     * it is possible to commit.
     */
    lcn_bool_t directory_owner;

    /**
     * Whether to close directory on closing the reader.
     */
    lcn_bool_t close_directory;

    /**
     * Hash containing norms, used by both SegmentReader
     * and MultiReader
     */
    apr_hash_t *norms;
    
    lcn_bool_t is_modified;
    
    lcn_bool_t is_directory_reader;

    /**
     * Returns true, if the index has been modified
     */
    lcn_bool_t (*has_changes) ( lcn_index_reader_t *index_reader);
    unsigned int (*num_docs) ( lcn_index_reader_t *index_reader );
    unsigned int (*max_doc) ( lcn_index_reader_t *index_reader );
    apr_status_t (*doc_freq) ( lcn_index_reader_t *index_reader,
                               const lcn_term_t *term,
                               int *freq );

    lcn_bool_t (*has_deletions) ( lcn_index_reader_t *index_reader );
    lcn_bool_t (*has_norms) ( lcn_index_reader_t *index_reader, const char *field );

    apr_status_t (*get_norms)( lcn_index_reader_t* index_reader,
                               lcn_byte_array_t** norms,
                               const char* field );

    apr_status_t (*read_norms_to_array)( lcn_index_reader_t* index_reader,
                                         lcn_byte_array_t* norms,
                                         unsigned int offset,
                                         const char* field );

    apr_status_t (*do_delete) (lcn_index_reader_t *index_reader, int doc_num );
    apr_status_t (*do_undelete_all) (lcn_index_reader_t *index_reader );
    apr_status_t (*do_commit) (lcn_index_reader_t *index_reader );
    apr_status_t (*do_close) (lcn_index_reader_t *index_reader );
    lcn_bool_t (*is_deleted) (lcn_index_reader_t *index_reader, unsigned int n );
    apr_status_t (*document) (lcn_index_reader_t *index_reader,
                              lcn_document_t **document,
                              unsigned int n,
                              apr_pool_t *pool );

    apr_status_t (*terms) (lcn_index_reader_t *index_reader,
                           lcn_term_enum_t **term_enum,
                           apr_pool_t *pool );

    apr_status_t (*terms_from) (lcn_index_reader_t *index_reader,
                                lcn_term_enum_t **term_enum,
                                lcn_term_t *term,
                                apr_pool_t *pool );

    apr_status_t (*term_docs) ( lcn_index_reader_t *index_reader,
                                lcn_term_docs_t **term_docs,
                                apr_pool_t *pool );

    apr_status_t (*term_docs_from) ( lcn_index_reader_t *index_reader,
                                     lcn_term_docs_t **term_docs,
                                     const lcn_term_t *term,
                                     apr_pool_t *pool );

    apr_status_t (*term_positions) ( lcn_index_reader_t *index_reader,
                                     lcn_term_docs_t **term_positions,
                                     apr_pool_t *pool );

    apr_status_t (*get_field_infos) ( lcn_index_reader_t *index_reader,
                                      lcn_list_t *field_infos,
                                      unsigned int field_flags,
                                      unsigned int options_mask );

    apr_status_t (*fs_field_bitvector) ( lcn_index_reader_t *reader,
                                         lcn_bitvector_t **bitvector,
                                         const char *fname,
                                         lcn_bool_t (*filter_function)( lcn_fs_field_t *, unsigned int ),
                                         apr_pool_t *pool );


    apr_status_t (*fs_field_int_bitvector) ( lcn_index_reader_t *reader,
                                             lcn_bitvector_t **bitvector,
                                             const char *fname,
                                             lcn_bool_t (*filter_function) (void* data, unsigned int val, unsigned int doc_order ),
                                             void *filter_data,
                                             apr_pool_t *pool );

    apr_status_t (*set_int_value) ( lcn_index_reader_t *reader,
                                    unsigned int docid,
                                    const char *fname,
                                    unsigned int int_value );

    apr_status_t (*set_char_value) ( lcn_index_reader_t *reader,
                                     unsigned int docid,
                                     const char *fname,
                                     const char *char_value );

    apr_status_t (*add_fs_field_def) ( lcn_index_reader_t *reader,
                                       lcn_field_t *field );

    FILE *log_stream;
};

typedef struct lcn_composite_reader_t {
    struct lcn_index_reader_t index_reader;
} lcn_composite_reader_t;

typedef struct lcn_base_composite_reader_atomic_t {
    struct lcn_composite_reader_t composite_reader;
} lcn_base_composite_reader_atomic_t;

/**
 * @brief  Reader for joining multiple readers together
 *
 * Used for reading non optimized indexes or for composing
 * more indexes to a single one
 */
typedef struct lcn_multi_reader_t {

    /**
     * Base class of multi reader
     */
    lcn_index_reader_t parent;

    /**
     * Readers composing the multi reader
     */
    lcn_index_reader_t **sub_readers;

    /**
     * Number of sub_readers composing the multi reader
     */
    unsigned int sub_readers_count;

    /**
     * Start id of different segments of the reader
     */
    unsigned int *starts;

    /**
     * Holds calculated max_doc of the multi reader
     */
    unsigned int max_doc_count;

    /**
     * Stores calculated number of docs of the multi reader.
     * Deleted docs are not counted.
     */
    unsigned int num_docs_count;

    /**
     * Flag indicating there were deletions in the index.
     */
    lcn_bool_t has_deletions_flag;

} lcn_multi_reader_t;

/**
 * @brief Reader for single segments
 *
 * Is usede for optimized indexes or for every single segment
 * of non optimized indexes.
 */
typedef struct lcn_segment_reader_t {

    /**
     * Base class of segment reader
     */
    lcn_index_reader_t parent;

    /**
     * Name of the underlying index segment (e.g. _123)
     */
    const char *segment;

    /**
     * Vector containging deleted docs. A bit belonging
     * to a deleted document is set to 1.
     */
    lcn_bitvector_t *deleted_docs;

    /**
     * APR pool for managing bitvector for deleted docs.
     * Is cleared, if all documents are undeleted.
     */
    apr_pool_t *del_vector_pool;

    /**
     * Flag to indicate that there were documents deleting via this
     * segment reader
     */
    lcn_bool_t deleted_docs_dirty;

    /**
     * Flag to indicate that all documents were undeleted
     */
    lcn_bool_t undelete_all;

    /* TODO: not implemented yet: lcn_bool_t norms_dirty; */

    /**
     * Field infos of the underlying index
     */
    lcn_field_infos_t *field_infos;

    /**
     * Input stream for proximity information
     */
    lcn_istream_t *prox_stream;

    /**
     * Input stream for frequency information
     */
    lcn_istream_t *freq_stream;

    /**
     * Term infos reader
     */
    lcn_term_infos_reader_t *tis;

    /**
     * Fields reader
     */
    lcn_fields_reader_t *fields_reader;

} lcn_segment_reader_t;

#define lcn_cast_index_reader( reader )  ((lcn_index_reader_t*)(reader))

/**
 * @brief Initializes the base class of index readers
 *
 * @param index_reader     The reader to be initialized
 * @param directory        Underlying directory
 * @param segment_infos    Underlying segment infos
 * @param close_directory  Whether to close directory on closing index
 * @param directory_owner  Whether to own directory (important for commits)
 * @param pool             APR pool
 */
apr_status_t
lcn_index_reader_init( lcn_index_reader_t *index_reader,
                       lcn_directory_t *directory,
                       lcn_segment_infos_t *segment_infos,
                       lcn_bool_t close_directory,
                       lcn_bool_t directory_owner,
                       apr_pool_t *pool );

/**
 * @brief Create a multi reader using multiple subreaders
 *
 * @param index_reader     Stores the new index reader
 * @param directory        Underlying directory
 * @param segment_infos    Underlying segment infos
 * @param close_directory  Whether to close directory on closing index
 * @param readers          Subreaders
 * @param pool             APR pool
 */
apr_status_t
lcn_multi_reader_create( lcn_index_reader_t **index_reader,
                         lcn_directory_t *directory,
                         lcn_segment_infos_t *segment_infos,
                         lcn_bool_t close_directory,
                         lcn_index_reader_t **readers,
                         apr_pool_t *pool );

/**
 * @brief  Create a segment reader by a single segment info
 *
 * Is used for handling indexes with multiple segments
 *
 * @param index_reader  Stores new index reader
 * @param segment_info  Segment information
 * @param pool          APR pool
 */
apr_status_t
lcn_segment_reader_create_by_info( lcn_index_reader_t **index_reader,
                                   lcn_segment_info_t *segment_info,
                                   apr_pool_t *pool );

/**
 * @brief Creates segment reader
 *
 * A segment reader is a reader based on a single segment. Multiple
 * segment readers can be used to create a multy segment reader.
 *
 * @param index_reader   Stores new index reader
 * @param directory      Underlying directory
 * @param segment_infos  Underlying segment infos
 * @param segment_info   Segment info of the segment to use
 * @param close_dir      Whether to close directory on closing the reader
 * @param own_dir        Whether the reader is the owner of its directory. See #lcn_index_reader_create_as_owner.
 * @param pool           APR pool
 */
apr_status_t
lcn_segment_reader_create( lcn_index_reader_t **index_reader,
                           lcn_directory_t *directory,
                           lcn_segment_infos_t *segment_infos,
                           lcn_segment_info_t *segment_info,
                           lcn_bool_t close_dir,
                           lcn_bool_t own_dir,
                           apr_pool_t *pool );

/**
 * @brief  Creates an index reader owning its directory
 *
 * Index reader being owner of its directory means that
 * the reader can commit changes to index, e.g. deleting
 * documents. If an index reader does not own its directory,
 * those changes will be lost, they cannot be committed.
 *
 * @param index_reader     Index_reader to be initialized
 * @param directory        Underlying directory if the reader
 * @param segment_infos    Underlying segment infos
 * @param close_directory  Whether to close directory on closing the reader
 * @param pool             APR pool
 */
apr_status_t
lcn_index_reader_create_as_owner( lcn_index_reader_t *index_reader,
                                  lcn_directory_t *directory,
                                  lcn_segment_infos_t *segment_infos,
                                  lcn_bool_t close_directory,
                                  apr_pool_t *pool );

/** @} */

/**
 * @defgroup lcn_segment_merge_queue_priv SegmentMergeQueue
 * @ingroup lucene_index_priv
 * @{
 */

/**
 * @brief Creates a segment merge queue
 *
 * This is a specialized priority queue for sorting segment merge infos.
 * Furthermore it is used for term enumeration over multiple
 * segments.
 *
 * @param priority_queue   Stores the new queue
 * @param size             Size of new queue
 * @param pool             APR pool
 */
apr_status_t
lcn_segment_merge_queue_create( lcn_priority_queue_t** priority_queue,
                                unsigned int size,
                                apr_pool_t* pool );

/**
 * @brief Closes the segment merge queue
 *
 * Closes multiple term enumerations needed to implement a
 * term enumeration over multiple segments
 *
 * @param queue   Queue to be closed
 */
apr_status_t
lcn_segment_merge_queue_close( lcn_priority_queue_t *queue );

/** @} */

/**
 * @brief  Adds fs_fields to a document
 *
 * These fields are loaded on demand via a field accessor
 *
 * @param index_reader  Underlying index reader
 * @param document      Document for adding fields
 * @param n             Internal id of the document
 * @param pool          APR pool
 */
apr_status_t
lcn_index_reader_add_fs_fields( lcn_index_reader_t *index_reader,
                                lcn_document_t *document,
                                unsigned int n,
                                apr_pool_t *pool );

apr_status_t
lcn_segment_reader_user_compound_file( lcn_segment_info_t *segment_info, 
                                       lcn_bool_t *flag,
                                       apr_pool_t *pool);

/** @} */


#endif /* INDEX_READER_H */
