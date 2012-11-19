#ifndef LCN_BITVECTOR_H
#define LCN_BITVECTOR_H

#include "lucene.h"
#include "lcn_search.h"

BEGIN_C_DECLS

/**
 ** Constructs a vector capable of holding <code>n</code> bits.
 */

apr_status_t
lcn_bitvector_create ( lcn_bitvector_t** bitvector,
                       unsigned int size,
                       apr_pool_t* pool );

/**
 * A null bitvector is a constant bitvector with all bits 0.
 */
apr_status_t
lcn_null_bitvector_create( lcn_bitvector_t **bitvector,
                           unsigned int size,
                           apr_pool_t *pool );

apr_status_t
lcn_one_bitvector_create( lcn_bitvector_t** bitvector,
                          unsigned int size,
                          apr_pool_t* pool );
/**
 * Reads a bitvector from the lcn_index_input_t.
 */
apr_status_t
lcn_bitvector_from_stream ( lcn_bitvector_t **new_bv,
                            lcn_index_input_t *in,
                            apr_pool_t* pool );

apr_status_t
lcn_bitvector_create_by_bits ( lcn_bitvector_t** bitvector,
                               unsigned int size,
                               char *bits,
                               apr_pool_t* pool );

/**
 * @brief Create lazy bitvector delegating get bit -Operation
 *        to the underlying field, which must be an integer fixed
 *        sized field.
 *
 * @param bitvector     Newly created bitvector
 * @param field         Fixed sized field with integer values
 * @param filter_value  Value for which to set one bits
 * @param pool          APR pool
 */
apr_status_t
lcn_bitvector_create_by_int_fs_field ( lcn_bitvector_t** bitvector,
                                       lcn_fs_field_t *field,
                                       unsigned int filter_value,
                                       apr_pool_t* pool );

apr_status_t
lcn_bitvector_alloc_bits( lcn_bitvector_t *bitvector );

char *
lcn_bitvector_bits( const lcn_bitvector_t *bitvector );

apr_status_t
lcn_bitvector_dump_file( lcn_bitvector_t* bitvector,
                         const char *name,
                         apr_pool_t* pool );

/**
 * Reads a bitvector from the file in a given lcn-Directory
 *
 * @param bitvector Result
 * @param directory Lucene-Directory
 * @param name Filename
 * @param pool Memory-Context
 *
 * @result lucene-statusvalue
 */

apr_status_t
lcn_bitvector_from_dir( lcn_bitvector_t** bitvector,
                        lcn_directory_t*  directory,
                        const char* name,
                        apr_pool_t* pool );

/**
 ** Returns the total number of one bits in this vector.  This is efficiently
 ** computed and cached, so that, if the vector is not changed, no
 ** recomputation is done for repeated calls.
 */
unsigned int
lcn_bitvector_count ( lcn_bitvector_t *bitvector );

apr_status_t
lcn_bitvector_not( lcn_bitvector_t* bitvector );

apr_status_t
lcn_bitvector_and( lcn_bitvector_t **result,
                   lcn_bitvector_t *bv_a,
                   lcn_bitvector_t *bv_b,
                   apr_pool_t *pool );

apr_status_t
lcn_bitvector_uand( lcn_bitvector_t *bv_a,
                    lcn_bitvector_t *bv_b );

apr_status_t
lcn_bitvector_uor( lcn_bitvector_t *bv_a,
                   lcn_bitvector_t *bv_b );

apr_status_t
lcn_bitvector_create_by_concat( lcn_bitvector_t **result,
                                lcn_list_t *bv_list,
                                apr_pool_t *pool );

apr_status_t
lcn_bitvector_or( lcn_bitvector_t **result,
                  lcn_bitvector_t *bv_a,
                  lcn_bitvector_t *bv_b,
                  apr_pool_t *pool );

/**
 * Compare two lcn_bitvector_ts.
 *
 * @param bv One bit vector
 * @param compare The second to compare
 *
 * @return TRUE, if equal, FALSE else
 */

lcn_bool_t
lcn_bitvector_equals ( lcn_bitvector_t *bitvector_a, lcn_bitvector_t *bitvector_b );

/**
 * Returns the value of the n-th bit
 *
 * @param bitvector Bitvector
 * @param nth Number of the bit
 *
 * @result Value of the nth bit
 */

lcn_bool_t
lcn_bitvector_get_bit( lcn_bitvector_t* bitvector, unsigned int nth );

/**
 * Returns the the number of bits in the vector
 *
 * @param bitvector Bitvector
 * @param nth Number of the bit
 *
 * @result Value
 */

unsigned int
lcn_bitvector_size( const lcn_bitvector_t* bitvector );

/**
 * Sets the nth bit to one
 *
 * @param bitvector Bitvector
 * @param nth The Bit to set
 */

apr_status_t
lcn_bitvector_set_bit( lcn_bitvector_t* bitvector, unsigned int bit );

/**
 * Sets the nth bit to zero
 *
 * @param bitvector Bitvector
 * @param nth The Bit to clear
 */

void
lcn_bitvector_clear_bit( lcn_bitvector_t* bitvector, unsigned int bit );

apr_status_t
lcn_bitvector_clone( lcn_bitvector_t **clone,
                     lcn_bitvector_t *bitvector,
                     apr_pool_t *pool );


/**
 * Writes Bitvector to file
 *
 * @param bitvector Bitvector
 * @param path Path to the Outputfile
 * @param pool Memory-Context

 * @result lcn-statusvalue
 */

apr_status_t
lcn_bitvector_write_file ( lcn_bitvector_t *bitvector,
                           const char *path,
                           apr_pool_t* pool );

/**
 * Writes Bitvector into a file in a Directory
 *
 * @param bitvector Bitvector
 * @param dir lcn-Directory
 * @param name Name of the file
 * @param pool Memory-Context
 *
 * @result lcn-statusvalue
 */

apr_status_t
lcn_bitvector_write ( lcn_bitvector_t *self,
                      lcn_directory_t *dir,
                      const char *name,
                      apr_pool_t* pool );

apr_status_t
lcn_query_bitvector_create( lcn_bitvector_t** bitvector,
                            lcn_query_t* query,
                            lcn_searcher_t* searcher,
                            lcn_bitvector_t *search_filter,
                            apr_pool_t* pool );


apr_status_t
lcn_query_bitvector_create_by_term_list( lcn_bitvector_t** bitvector,
                                         lcn_list_t *term_list,
                                         lcn_searcher_t* searcher,
                                         apr_pool_t* pool );

apr_status_t
lcn_bitvector_create_from_file ( lcn_bitvector_t** bitvector,
                                 const char *file_name,
                                 apr_pool_t* pool );



END_C_DECLS

#endif /* LCN_BITVECTOR_H */
