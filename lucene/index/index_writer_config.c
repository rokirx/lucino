#include "lucene.h"
#include "index_writer_config.h"

struct _lcn_index_writer_config_t {
    
    unsigned int open_mode;
    
    unsigned int default_max_buffered_docs;

};

apr_status_t
lcn_index_writer_config_create ( lcn_index_writer_config_t **iwc,
                                 apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        LCNPV( *iwc = apr_pcalloc( pool, sizeof ( lcn_index_writer_config_t ) ), LCN_ERR_NULL_PTR );
        lcn_index_writer_config_set_open_mode( *iwc, LCN_INDEX_WRITER_CONFIG_OPEN_MODE_APPEND_OR_CREATE );
        lcn_index_writer_config_set_max_buffered_docs( *iwc, LCN_INDEX_WRITER_CONFIG_DEFAULT_MAX_BUFFERED_DOCS );
    }
    while ( 0 );

    return s;
}

void
lcn_index_writer_config_set_open_mode ( lcn_index_writer_config_t *iwc,
                                        unsigned int open_mode )
{
    iwc->open_mode = open_mode;
}

unsigned int
lcn_index_writer_config_get_open_mode ( lcn_index_writer_config_t *iwc )
{
    return iwc->open_mode;
}

void
lcn_index_writer_config_set_max_buffered_docs ( lcn_index_writer_config_t *iwc,
                                                unsigned int default_max_buffered_docs )
{
    iwc->default_max_buffered_docs = default_max_buffered_docs;
}

unsigned int
lcn_index_writer_config_get_max_buffered_docs ( lcn_index_writer_config_t *iwc )
{
    return iwc->default_max_buffered_docs;
}