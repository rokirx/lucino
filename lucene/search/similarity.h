#ifndef SIMILARITY_H
#define SIMILARITY_H

#include "lcn_search.h"

struct lcn_similarity_t
{
    lcn_byte_t (*encode_norm) ( float f );
    float      (*decode_norm) ( lcn_byte_t b );
    float      (*length_norm) ( const char* field_name,
                                unsigned int num_tokens );
    float      (*query_norm)  ( float sum_of_squared_weight );
    float      (*tf)          ( float freq );
    float      (*sloppy_freq) ( unsigned int distance );
    float      (*idf)         ( unsigned int doc_freq, unsigned int num_docs );
    float      (*coord)       ( unsigned int overlap, unsigned int max_overlap );
    float*     norm_decoder;
};

#endif /* SIMILARITY_H */

