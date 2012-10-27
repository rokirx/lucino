#ifndef COMPOUND_FILE_UTIL_H
#define	COMPOUND_FILE_UTIL_H

#define CP_EXT_COUNT (7)

extern char* COMPOUND_EXTENSIONS[];

lcn_bool_t
lcn_compound_file_util_check_file_extension ( char* filename );

apr_status_t
lcn_compound_file_util_compound_file_exists( lcn_directory_t *dir,
                                             const char *file_name,
                                             lcn_bool_t *flag,
                                             apr_pool_t *pool);

#endif	/* COMPOUND_FILE_UTIL_H */

