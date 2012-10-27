#include "lucene.h"
#include "term.h"

LUCENE_EXTERN apr_status_t
LUCENE_API(lcn_term_create) ( lcn_term_t **new_term,
                              const char *field,
                              const char *text,
                              int text_copy_flag,
                              apr_pool_t *pool )
{
    if ( NULL == ( *new_term = (lcn_term_t*) apr_palloc( pool, sizeof(lcn_term_t) )))
    {
        return APR_ENOMEM;
    }

    (*new_term)->text  = ( text_copy_flag == LCN_TERM_NO_TEXT_COPY ? (char*) text : apr_pstrdup( pool, text ) );
    (*new_term)->field = lcn_atom_get_str( field );

    return APR_SUCCESS;
}

/**
 * The lcn_term_compare function compares the two terms t1 and t2.
 * It returns an integer less than, equal to, or greater than zero
 * if t1 is found, respectively, to be less than, to match, or be
 * greater than t2.
 */
int
lcn_term_compare( const lcn_term_t *t1, const lcn_term_t *t2 )
{
    if ( t1->field == t2->field )
    {
        return strcmp( t1->text, t2->text );
    }

    return strcmp( t1->field, t2->field );
}

apr_status_t
lcn_term_clone( const lcn_term_t* term,
                lcn_term_t** clone,
                apr_pool_t* pool )
{
    return lcn_term_create( clone,
                            lcn_term_field( term ),
                            lcn_term_text( term ),
                            LCN_TERM_TEXT_COPY,
                            pool );
}

const char *
lcn_term_text( const lcn_term_t *term )
{
    return term->text;
}

const char *
lcn_term_field( const lcn_term_t *term )
{
    return term->field;
}

void
lcn_term_set( lcn_term_t *term,
              const char *field_name,
              const char *term_text )
{
    term->text = (char*) term_text;
    term->field = field_name;
}


apr_status_t
lcn_term_info_create( lcn_term_info_t **term_info,
                      unsigned int doc_freq,
                      apr_off_t freq_pointer,
                      apr_off_t prox_pointer,
                      apr_off_t skip_offset,
                      apr_pool_t *pool )
{
    if ( NULL == ((*term_info)=(lcn_term_info_t*)apr_palloc(pool, sizeof(lcn_term_info_t))))
    {
        return APR_ENOMEM;
    }

    (*term_info)->doc_freq = doc_freq;
    (*term_info)->freq_pointer = freq_pointer;
    (*term_info)->prox_pointer = prox_pointer;
    (*term_info)->skip_offset = skip_offset;

    return APR_SUCCESS;
}
