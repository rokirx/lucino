#ifndef STEMMER_H
#define STEMMER_H

#include "lcn_analysis.h"

struct lcn_stemmer_t
{
    unsigned int subst_count;

    void (*stem)( lcn_stemmer_t* stemmer, char* word );
    char* buf;

    apr_pool_t* pool;
};

#endif /* STEMMER_H */
