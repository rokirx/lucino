#ifndef FS_FIELD_H
#define FS_FIELD_H

#include "lcn_index.h"
#include "field.h"

#define LCN_DIRECTORY_FS_FIELD (0)
#define LCN_MULTI_FS_FIELD     (1)


/**
 * @defgroup lcn_fixed_sized_field_priv Fixed Size Field
 * @ingroup lcn_index_priv
 * @{
 */

/**
 * @brief Lucene fixed size field object
 *
 * The fixed size field can be used to store the field contents of
 * fixed length. A use case for such fields are dates, date ranges,
 * sort keys, --- generally any numeric data about a document or short
 * strings of fixed length.
 *
 * The size of the field is given in bits. This enables very efficient
 * storage of short data. If the length is one bit, then fixed size can be used
 * as a filter.
 *
 * The data is passed as char*, the bit numbering is as follows:
 *
 * \code
 *  for char* data:
 *             bit number
 *            7 6 5 4 3 2 1 0
 *
 *  data[0]:  x x x x x x x x
 *  data[1]:  x x x x x x x x
 *  data[2]:  x x x x x x x x
 *  ....
 * \endcode
 *
 * so the 11th bit of a data is the bit 3 of data[1], starting counting from 0.
 *
 * An alternative is passing the data as integer, which is possible up to 32 bits,
 * which is enough for numerical data in most cases.
 */
struct lcn_fs_field_t {

    /**
     * Field accessor for lazy loading of fields
     */
    lcn_field_accessor_t accessor;

    /**
     * APR pool
     */
    apr_pool_t *pool;

    /**
     * Name of the field. Used for file names to store
     * the field data. The file extenstion is ".fsf".
     */
    char *name;

    /**
     * Number of docs in the field
     */
    unsigned int docs_count;

    /**
     * Bit size of the field data. Must be at least 1
     */
    unsigned int data_size;

    /**
     * Default value of the field. Can be NULL, in this case
     * a data with all 0 bits is considered the default_value
     */
    char *default_value;

    /**
     * Used for implementation of set to avoid allocating
     * memory
     */
    char *tmp_value;

    /**
     * True if there were some modified values
     */
    lcn_bool_t is_modified;

    /**
     *
     */
    int type;

    /**
     * @brief Retrieves the field value of the document given by doc_id
     *
     * IMPORTANT: The size of val buffer must be big enough to hold the field data
     *
     * @param field    The field object
     * @param val      Stores the field value of the document
     * @param doc_id   The internal doc id of the document
     */
    apr_status_t (*field_value) ( lcn_fs_field_t *field, char *val, unsigned int doc_id );

    /**
     * @brief Retrieves the field value of the document as int
     *
     * @param field    The field object
     * @param val      Integer value of the field
     * @param doc_id   The internal doc id of the document
     */
    apr_status_t (*int_value) ( lcn_fs_field_t *field, unsigned int *val, unsigned int doc_id );

    /**
     * @brief  Sets the integer field data for the given document
     *
     * Only #lcn_fs_field_data_size of the integer are relevant
     *
     * @param   field   The field object
     * @param   val     Integer field value
     * @param   doc_id  The internal doc id of the document
     */
    apr_status_t (*set_int_value) ( lcn_fs_field_t *field, unsigned int val, unsigned int doc_id );

    /**
     * @brief  Sets the field data for the given document
     *
     * @param   field   The field object
     * @param   val     Field value
     * @param   doc_id  The internal doc id of the document
     */
    apr_status_t (*set_value) ( lcn_fs_field_t *field, const char *val, unsigned int doc_id );

    /**
     * @brief Writes a possibly changed or new fixed size
     *        field to the field directory. It also updates
     *        the definition file of the fixed size fields
     *        in the directory
     *
     * @param field
     * @param pool
     */
    apr_status_t (*commit) ( lcn_fs_field_t *field, apr_pool_t *pool );

    /**
     * @brief Writes a possibly changed or new fixed size
     *        field to the field directory and closes it. It also updates
     *        the definition file of the fixed size fields in the directory
     *
     * @param field
     * @param pool
     */
    apr_status_t (*close)  ( lcn_fs_field_t *field, apr_pool_t *pool );
};

typedef struct lcn_multi_fs_field_t {

    /**
     * Base type
     */
    lcn_fs_field_t parent;

    /**
     * List of subfields wich the multi field is composed of
     */
    lcn_list_t* sub_fields;

    /**
     * List of field offsets
     */
    unsigned int* offsets;

} lcn_multi_fs_field_t;

typedef struct lcn_directory_fs_field_t {

    /**
     * Base type
     */
    lcn_fs_field_t parent;

    /**
     * Size of the internal buffer for storing field values
     */
    unsigned int buf_size;

    /**
     * Internal buffer for storing field values
     */
    char* buf;

    /**
     * Underlying directory for lazy loading of fields
     */
    lcn_directory_t *directory;

    /**
     * Base offset for calculating correct file position
     */
    apr_off_t base_offset;

    /**
     * Stream for lazy loading field values
     */
    lcn_index_input_t *istream;

} lcn_directory_fs_field_t;



/* fs_field functions suitable for multi-fields */

/**
 * @brief Returns true if the field has been modified
 *
 * @param fs_field   Fixed sized field
 */
lcn_bool_t
lcn_fs_field_is_modified( lcn_fs_field_t* fs_field );

/**
 * @brief Creates a field (#lcn_field_t) with a lazy access
 *
 * @param fs_field    Original fixed size field
 * @param field       Stores the new field
 * @param doc_id      Internal id of the document
 * @param pool        APR pool
 */
apr_status_t
lcn_fs_field_to_field( lcn_fs_field_t* fs_field,
                       lcn_field_t **field,
                       unsigned int doc_id,
                       apr_pool_t *pool );

/**
 * @brief  Sets the field data for the given document
 *
 * @param   field   The field object
 * @param   val     Field value
 * @param   doc_id  The internal doc id of the document
 */
apr_status_t
lcn_fs_field_set_value( lcn_fs_field_t *field,
                        const char *val,
                        unsigned int doc_id );


/**
 * @brief  Sets the integer field data for the given document
 *
 * Only #lcn_fs_field_data_size of the integer are relevant
 *
 * @param   field   The field object
 * @param   val     Integer field value
 * @param   doc_id  The internal doc id of the document
 */
apr_status_t
lcn_fs_field_set_int_value( lcn_fs_field_t *field,
                            unsigned int val,
                            unsigned int doc_id );

/**
 * @brief Default value of the field
 *
 * Is used while indexing to check consistency of the
 * field definitions
 *
 * @param field   Underlying field
 * @return  The default value of the field. The size
 *          of the buffer can be calculated from
 *          lcn_fs_field_data_size.
 */
const char*
lcn_fs_field_default_val( const lcn_fs_field_t *field );







/* following functions are not implemented for multi-fs-fields  */
/* they return LCN_ERR_UNSUPPORTED_OPERATION on multi-fs_fields */

/**
 * @brief Reads field contents from an input stream.
 *
 * Does not close the istream after reading
 *
 * @param field    Stores new field
 * @param name     Name of the field
 * @param istream  Input stream to read the contents from
 * @param pool     APR pool
 */
apr_status_t
lcn_directory_fs_field_read( lcn_directory_fs_field_t **field,
                             const char *name,
                             lcn_index_input_t *istream,
                             apr_pool_t *pool );

/**
 * @brief Writes field contents to an output stream
 *
 * The format of the written data is as follows
 *
 * \code
 * int     : integer with the version file format number
 * string  : name of the field
 * int     : number of documents in the field (N)
 * int     : size of the field data in bits   (b)
 * the following b/8 + (b&7 ? 1: 0) bytes:
 *      bytewise stored default value
 * \endcode
 *
 * Does not close the stream after writing
 *
 * @param field    Field to be written
 * @param ostream  Output stream for writing
 */
apr_status_t
lcn_fs_field_write_info( lcn_directory_fs_field_t *field,
                         lcn_ostream_t *ostream );

/**
 * @brief Writes field contents to an output stream
 *
 * The format of the written data is as follows
 *
 * \code
 * (N * b) / 8 + (((N*b) & 7) ? 1 : 0) bytes :
 *      bytewise stored field data
 *
 *  where N is the number of documents in the field and
 *  b ist the size of the field data in bits
 * \endcode
 *
 * Does not close the stream after writing
 *
 * @param field    Field to be written
 * @param ostream  Output stream for writing
 */
apr_status_t
lcn_fs_field_write_content( lcn_directory_fs_field_t *field,
                            lcn_ostream_t *ostream );

/**
 * @brief  Creates a new fixed size field
 *
 * Directory fs field is a field used for all segments in
 * a directory: there is one file for all segments, and
 * there is no need to build a hierarchy of fields.
 *
 * @param field          Stores new field
 * @param name           Name of the field
 * @param docs_count     Number of documents stored
 * @param field_size     Bit size of a single field
 * @param dir            Directory of the field
 * @param pool           APR pool
 */
apr_status_t
lcn_directory_fs_field_create( lcn_fs_field_t **field,
                               const char *name,
                               unsigned int docs_count,
                               unsigned int field_size,
                               lcn_directory_t *dir,
                               apr_pool_t *pool );

/**
 * @brief  Creates a new lazy fixed size field
 *
 * Lazy refers to the initialization of the field
 * components postponed till the first field (read)
 * access.
 *
 * @param field          Stores new field
 * @param name           Name of the field
 * @param docs_count     Number of documents stored
 * @param field_size     Bit size of a single field
 * @param pool           APR pool
 */
apr_status_t
lcn_fs_field_create( lcn_fs_field_t **field,
                     const char *name,
                     unsigned int docs_count,
                     unsigned int field_size,
                     apr_pool_t *pool );

/**
 * @brief Creates a multi_fs_field containing a given sub_field as base
 *
 * @param field        Resulting new field
 * @param sub_fields   Underlying fs_fields
 * @param offsets      Start index of the underlying start field
 * @param pool         APR pool
 */
apr_status_t
lcn_multi_fs_field_create_by_subfields( lcn_fs_field_t **field,
                                        lcn_list_t *sub_fields,
                                        unsigned int *offsets,
                                        apr_pool_t *pool );

/**
 * @brief  Set default integer field value
 *
 * @param field         The field object
 * @param default_value The default value
 */
apr_status_t
lcn_directory_fs_field_set_default_int_value( lcn_directory_fs_field_t *field,
                                              unsigned int default_value );

/**
 * @brief Set default field value
 *
 * @param field          The field object
 * @param default_value  The default value
 */
apr_status_t
lcn_directory_fs_field_set_default_value( lcn_directory_fs_field_t *field,
                                          const char *default_value );

/**
 * @brief  Reads field infos into a hash
 *
 * Stores fixed size fields (#lcn_fs_field_t) in a hash
 *
 * @param hash      The hash to store the fields
 * @param directory directory to read the field infos from
 * @param pool      APR pool
 */
apr_status_t
lcn_directory_fs_field_read_field_infos( apr_hash_t *hash,
                                         lcn_directory_t *directory,
                                         apr_pool_t *pool );

/**
 * @brief  Reads the field contents into internal buffer
 *
 * @param field   The field to read
 */
apr_status_t
lcn_directory_fs_field_init_buffer( lcn_directory_fs_field_t *field );

/**
 * @brief Converts an int value to a character buffer
 *
 * The character buffer must have enough space allocated
 * to hold the required number of bytes, which depends on
 * the size of unsigned int.
 *
 * @param val       Integer to be converted
 * @param buf       Target buffer
 * @param bit_size  Number of relevant bits. Used to calculate the
 *                  significant bytes of val, not to mask the irrelevant
 *                  bits.
 */
apr_status_t
lcn_fs_field_int_to_char( unsigned int val,
                          char *buf,
                          unsigned int bit_size );


apr_status_t
lcn_fs_field_update_fields_def( lcn_directory_fs_field_t *field,
                                lcn_directory_t* directory,
                                apr_pool_t *pool );

lcn_bool_t
lcn_fs_field_info_is_equal( lcn_fs_field_t *fa,
                            lcn_fs_field_t *fb );

/**
 * @brief Converts default value from char  to int
 *
 * @param data_size      Number of relevant bits in default value
 * @param default_value  Default Value
 */
unsigned int
lcn_fs_field_default_to_int( unsigned int data_size,
                             char *default_value );

/**
 * @brief Updates value of the modified flag based on the values
 *        of the subfields
 *
 * @param f  Field object
 */
void
lcn_multi_fs_field_update_modified_flag( lcn_multi_fs_field_t* f );



/* @} */

#endif
