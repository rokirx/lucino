#ifndef STRING_BUFFER
#define STRING_BUFFER

#include "lcn_util.h"

#define LCN_STRING_BUFFER_BLOCK_SIZE ( 1024 )

typedef struct
{
    char* str;
    unsigned int len;

} lcn_string_buffer_entry_t;

struct lcn_string_buffer_t
{
    lcn_list_t* string_list;
    unsigned int length;
    unsigned int block_size;
    lcn_string_buffer_entry_t* last;
};

#endif /* STRING_BUFFER */
