#include "phrase_positions.h"
#include "term_docs.h"

apr_status_t
lcn_phrase_positions_next( lcn_phrase_positions_t* pp )
{
    apr_status_t s;

    if( APR_SUCCESS != ( s = lcn_term_docs_next( pp->tp ) ) )
    {

        if( ( LCN_ERR_NO_SUCH_DOC != s ) &&
            ( LCN_ERR_ITERATOR_NO_NEXT != s ) )
        {
            LCNRM( s, "Error getting next term_doc" );
        }
        LCNCR( lcn_term_docs_close( pp->tp ) );

        return LCN_ERR_NO_SUCH_DOC;
    }

    pp->doc = lcn_term_docs_doc( pp->tp );
    pp->position = 0;

    return APR_SUCCESS;
}

apr_status_t
lcn_phrase_positions_skip_to( lcn_phrase_positions_t* pp, apr_ssize_t target )
{
    apr_status_t s;

    if( APR_SUCCESS !=
        ( s = lcn_term_docs_skip_to( pp->tp, target ) ) )
    {
        LCNCR( lcn_term_docs_close( pp->tp ) );
        return LCN_ERR_NO_SUCH_DOC;
    }

    pp->doc = lcn_term_docs_doc( pp->tp );
    pp->position = 0;

    return APR_SUCCESS;
}

apr_status_t
lcn_phrase_positions_next_position( lcn_phrase_positions_t* pp )
{
    apr_status_t s;

    while( pp->count-- > 0 )
    {
        apr_ssize_t next_pos;

        LCNCR( lcn_term_positions_next_position( pp->tp, &next_pos ) );

        if ( next_pos >= pp->offset )
        {
            pp->position =  next_pos - pp->offset;
            return APR_SUCCESS;
        }
    }

    return LCN_ERR_NO_SUCH_DOC;
}

apr_status_t
lcn_phrase_positions_first_position( lcn_phrase_positions_t* pp )
{
    pp->count = lcn_term_docs_freq( pp->tp );
    return lcn_phrase_positions_next_position( pp );
}

apr_status_t
lcn_phrase_positions_create( lcn_phrase_positions_t** pp,
                             lcn_term_docs_t* term_positions,
                             apr_ssize_t offset,
                             apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *pp = apr_pcalloc( pool, sizeof( lcn_phrase_positions_t ) ), APR_ENOMEM );

        (*pp)->tp = term_positions;
        (*pp)->offset = offset;
    }
    while( FALSE );

    return s;
}


/** for debug purposes */

char*
lcn_phrase_positions_to_string( lcn_phrase_positions_t* pp,
                                apr_pool_t *pool )
{
    char *me = apr_psprintf( pool, "d: %d o: %d p: %d c: %d", (int) pp->doc, (int) pp->offset, (int) pp->position, (int) pp->count );

    if ( pp->next_repeating != NULL )
    {
        return apr_psprintf( pool, "%s rpt[ %s ]", me, lcn_phrase_positions_to_string( pp->next_repeating, pool ));
    }

    return me;
}
