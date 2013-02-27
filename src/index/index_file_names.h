#ifndef INDEX_FILE_NAMES_H
#define INDEX_FILE_NAMES_H

/** Name of the index segment file */

#define LCN_INDEX_FILE_NAMES_SEGMENTS ("segments")

/** Extension of gen file */
#define LCN_INDEX_FILE_NAMES_GEN_EXTENSION ("gen")

/** Name of the generation reference file name */
#define LCN_INDEX_FILE_NAMES_SEGMENTS_GEN  ("segments.gen")

/** java.lang.Character.MAX_RADIX **/
#define LCN_INDEX_FILE_NAMES_MAX_RADIX (36)

char* 
lcn_index_file_names_segment_file_name( const char *segment_name,
                                        const char *segment_suffix,
                                        const char *ext,
                                        apr_pool_t *pool );

char* 
lcn_index_file_names_file_name_from_generation( char *base,
                                                char *ext,
                                                apr_int64_t gen,
                                                apr_pool_t *pool );

#endif /* INDEX_FILE_NAMES_H */
