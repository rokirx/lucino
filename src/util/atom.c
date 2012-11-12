#include "lucene.h"

static apr_hash_t *lcn_atoms = NULL;
FILE* lcn_log_stream = NULL;


const char *
lcn_atom_get_str ( const char *str )
{
    char *atom;

    if ( NULL == ( atom = (char*) apr_hash_get( lcn_atoms, str, strlen(str) )))
    {
        atom = apr_pstrdup( apr_hash_pool_get( lcn_atoms ), str );
        apr_hash_set( lcn_atoms, atom, strlen( atom ), atom );
    }

    return atom;
}

apr_status_t
lcn_atom_init ( apr_pool_t *pool )
{
    if ( NULL == (lcn_atoms = apr_hash_make( pool )))
    {
        return APR_ENOMEM;
    }

    return APR_SUCCESS;
}
