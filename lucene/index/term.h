#ifndef TERM_H
#define TERM_H

#include "lucene.h"
#include "apr_pools.h"

#define TERM_BUFFER_INITIAL_SIZE 200

struct lcn_term_t {

    /**
     * Field of the term
     */
    const char *field;

    /**
     * Text of the term
     */
    char *text;
};

struct lcn_term_info_t {

    /**
     * frequency of term occurence in the document
     */
    unsigned int doc_freq;

    /**
     * pointer to the frequencies list of the term
     */
    apr_off_t freq_pointer;

    /**
     * pointer to the proximity list of the term
     */
    apr_off_t prox_pointer;

    /**
     * TODO
     */
    apr_off_t skip_offset;

};

void
lcn_term_set( lcn_term_t *term,
              const char *field_name,
              const char *term_text );


#endif
