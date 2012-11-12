#ifndef QUERY_TOKENIZER_H
#define QUERY_TOKENIZER_H

#include "lcn_search.h"

BEGIN_C_DECLS

#define LCNQ_EOS (100)

struct lcn_query_tokenizer_t {

    apr_pool_t *pool;
    char *text;
    char *eofbuf;

};

END_C_DECLS

#endif /* QUERY_TOKENIZER_H */
