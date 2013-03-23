#include "lucene.h"
#include "lcn_util.h"
#include "index_file_names.h"

/**
 * Returns a file name that includes the given segment name, your own custom
 * name and extension. The format of the filename is:
 *
 * <segmentName>(_<name>)(.<ext>)
 *
 * NOTE: .<ext> is added to the result file name only if
 * ext is not empty.
 *
 * NOTE: _<segmentSuffix> is added to the result file name only if
 * it's not the empty string
 *
 * NOTE: all custom files should be named using this method, or
 * otherwise some structures may fail to handle them properly (such as if they
 * are added to compound files).
 */
char*
lcn_index_file_names_segment_file_name( const char *segment_name,
                                        const char *segment_suffix,
                                        const char *ext,
                                        apr_pool_t *pool )
{
    lcn_bool_t has_ext    = ( NULL != ext && strlen(ext) > 0 );
    lcn_bool_t has_suffix = ( NULL != segment_suffix && strlen(segment_suffix) > 0 );

    if ( has_ext || has_suffix )
    {
        /* mayby TODO: assert !ext.startsWith("."); */

        return apr_pstrcat( pool,
                            segment_name,
                            has_suffix ? "_" : "",
                            has_suffix ? segment_suffix : "",
                            has_ext ? "." : "",
                            has_ext ? ext : "",
                            NULL );
    }
    else
    {
        return apr_pstrdup( pool, segment_name );
    }
}

/**
 * Computes the full file name from base, extension and generation.
 *
 * If the generation is -1, the file name is null.
 * If it's 0, the file name is <base>.<ext>
 * If it's > 0, the file name is <base>_<gen>.<ext>
 *
 * NOTE: .<ext> is added to the name only if ext not an empty string.
 *
 * @param base main part of the file name
 * @param ext extension of the filename
 * @param gen generation
 */
char*
lcn_index_file_names_file_name_from_generation( char *base,
                                                char *ext,
                                                apr_int64_t gen,
                                                apr_pool_t *pool )
{
    if ( gen == -1)
    {
        return NULL;
    }

    if ( gen == 0 )
    {
        return lcn_index_file_names_segment_file_name( base, "", ext, pool );
    }

    {
        /* mayby TODO: assert gen > 0 */

        /* max long encoded to base 36: 1z141z4 (length 7), so 10 */
        /* should be ok                                           */

        char gen_buf[10];
        lcn_bool_t has_ext = (NULL != ext && strlen(ext) > 0);

        lcn_itoa36( gen, gen_buf );

        return apr_pstrcat( pool,
                            base,
                            "_",
                            gen_buf,
                            has_ext ? "." : "",
                            has_ext ? ext : "",
                            NULL );
    }
}
