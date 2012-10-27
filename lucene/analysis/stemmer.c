#include "stemmer.h"

BEGIN_C_DECLS

void
lcn_stemmer_stem( lcn_stemmer_t* stemmer, char* word )
{
    stemmer->stem( stemmer, word );
}

END_C_DECLS

