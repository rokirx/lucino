#ifndef INDEX_WRITER_CONFIG_H
#define	INDEX_WRITER_CONFIG_H

#include "index_writer.h"


#define LCN_INDEX_WRITER_CONFIG_DISABLE_AUTO_FLUSH                     (-1)

/** 
 * Creates a new index or overwrites an existing one. 
 */
#define LCN_INDEX_WRITER_CONFIG_OPEN_MODE_CREATE                       (1)

/** 
 * Opens an existing index. 
 */
#define LCN_INDEX_WRITER_CONFIG_OPEN_MODE_APPEND                       (2)

/** 
 * Creates a new index if one does not exist,
 * otherwise it opens the index and documents will be appended. 
 */
#define LCN_INDEX_WRITER_CONFIG_OPEN_MODE_APPEND_OR_CREATE             (3)


#define LCN_INDEX_WRITER_CONFIG_DEFAULT_MAX_BUFFERED_DOCS              (LCN_INDEX_WRITER_CONFIG_DISABLE_AUTO_FLUSH)

apr_status_t
lcn_index_writer_config_create( lcn_index_writer_config_t **iwc,
                                apr_pool_t *pool );

void
lcn_index_writer_config_set_open_mode( lcn_index_writer_config_t *iwc,
                                       unsigned int open_mode );

unsigned int
lcn_index_writer_config_get_open_mode( lcn_index_writer_config_t *iwc );

void
lcn_index_writer_config_set_max_buffered_docs( lcn_index_writer_config_t *iwc,
                                               unsigned int default_max_buffered_docs );

unsigned int
lcn_index_writer_config_get_max_buffered_docs( lcn_index_writer_config_t *iwc );

#endif	/* INDEX_WRITER_CONFIG_H */

