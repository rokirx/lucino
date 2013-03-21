#include "lucene.h"
#include "index_writer.h"
#include "segment_infos.h"
#include "index_file_deleter.h"

void
lcn_index_file_deleter_create( lcn_index_writer_t* writer,
                               apr_pool_t *pool,
                               lcn_directory_t* dir,
                               lcn_segment_infos_t* segment_infos,                               
                               lcn_file_deleter_t **file_deleter )
{
    apr_status_t s;
    apr_pool_t *cp;
    
    do
    {
        char* current_segments_file = lcn_segment_infos_get_next_name()
        
        apr_pool_create( &cp, pool );
        
        
        LCNPV( *file_deleter = apr_pcalloc( pool, sizeof( lcn_file_deleter_t ) ), APR_ENOMEM );
        (*file_deleter)->writer = writer;
        (*file_deleter)->dir = dir;
        
        //TODO Weiter
    }
    while(0);
    
    return s;
}
