#ifndef INDEX_FILE_DELETER_H
#define	INDEX_FILE_DELETER_H

#include "segment_infos.h"
#include "index_writer.h"

typedef struct lcn_file_deleter_t_ 
{   
    lcn_index_writer_t *writer;
    
    lcn_directory_t *dir;       
    
} lcn_file_deleter_t ;

void
lcn_index_file_deleter_create( lcn_index_writer_t* writer,
                               apr_pool_t *pool,
                               lcn_directory_t* dir,
                               lcn_segment_infos_t* segment_infos,
                               lcn_file_deleter_t **file_deleter );

#endif
