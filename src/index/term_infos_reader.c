#include "lucene.h"
#include "lcn_index.h"
#include "lcn_term_enum.h"
#include "directory.h"
#include "term_infos_reader.h"
#include "term_enum.h"


static apr_status_t
lcn_term_infos_reader_seek_enum( lcn_term_infos_reader_t *ti_reader,
                                 apr_off_t index_offset )
{
    return lcn_term_enum_seek( ti_reader->orig_enum,
                               *(ti_reader->index_pointers + index_offset ),
                               ( index_offset * ti_reader->orig_enum->index_interval ) - 1,
                               *(ti_reader->index_terms + index_offset),
                               ti_reader->index_infos + index_offset );
}

static apr_off_t
lcn_term_infos_reader_get_index_offset( lcn_term_infos_reader_t *ti_reader,
                                        const lcn_term_t* term )
{
    /*  binary search indexTerms[] */

    apr_off_t delta;
    apr_off_t mid;
    apr_off_t lo = 0;
    apr_off_t hi = ti_reader->index_size - 1;

    while (hi >= lo) {
        mid = (lo + hi) >> 1;
        delta = lcn_term_compare( term, *(ti_reader->index_terms + mid));

        if (delta < 0){
            hi = mid - 1;
        } else if (delta > 0){
            lo = mid + 1;
        }else{
            return mid;
        }
    }

    return hi;
}

/**
 * Scans within block for matching term.
 */
static apr_status_t
lcn_term_infos_reader_scan_enum( lcn_term_infos_reader_t* ti_reader,
                                 lcn_term_info_t **term_info,
                                 const lcn_term_t *term )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        int equal;

        /**
         * We inline SegmentTermInfos.scanTo for two reasons:
         *
         * -- this function is used in Java only once: here
         * -- we needn't the optimizations (TermBuffer) like in Java
         *    because don't create term objects while scanning
         */
        while( APR_SUCCESS == s && lcn_term_compare( term, ti_reader->orig_enum->term ) > 0 )
        {
            s = lcn_term_enum_next( ti_reader->orig_enum );
        }

        if( LCN_ERR_ITERATOR_NO_NEXT == s )
        {
            s = APR_SUCCESS;
        }
        else if ( APR_SUCCESS != s )
        {
            break;
        }

        if ( ti_reader->orig_enum->term->field != NULL &&

             /* The second condition differs from that in Java. */
             /* We use weaker ( <= ) condition (in Java: == ),  */
             /* because in case of lacking exact match we want  */
             /* to retrieve the next term in the alphanumeric   */
             /* order.                                          */

             ((equal = lcn_term_compare( term, ti_reader->orig_enum->term )) <= 0) )
        {
            *term_info = ti_reader->orig_enum->term_info;

            if ( equal != 0 )
            {
                s = LCN_ERR_SCAN_ENUM_NO_MATCH;
            }
        }
        else
        {
            s = LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM;
        }
    }
    while(0);

    return s;
}


/* TODO: synchronized in Java */
static apr_status_t
lcn_term_infos_reader_ensure_index_is_read( lcn_term_infos_reader_t *ti_reader )
{
    apr_status_t s;
    apr_pool_t *pool;

    /* index already read, do nothing */
    if ( NULL != ti_reader->index_terms )
    {
        return APR_SUCCESS;
    }

    /* otherwise read index */

    LCNCR( apr_pool_create( &pool, ti_reader->pool ));

    do
    {
        unsigned int index_size;
        lcn_term_enum_t *index_enum;
        unsigned int i = 0;

        char *file_name;
        lcn_index_input_t *istream;

        LCNPV( file_name = apr_pstrcat( pool, ti_reader->segment, ".tii", NULL ), APR_ENOMEM );
        LCNCE( lcn_directory_open_input( ti_reader->dir, &istream, file_name, pool ) );
        LCNCE( lcn_segment_term_enum_create( &index_enum, istream, ti_reader->field_infos, LCN_TRUE, pool ));

        index_size = (unsigned int) lcn_term_enum_size( index_enum );

        LCNPV( ti_reader->index_terms = (lcn_term_t **) apr_palloc( ti_reader->pool, index_size * sizeof(lcn_term_t *) ),
               APR_ENOMEM );

        LCNPV( ti_reader->index_infos = (lcn_term_info_t *) apr_palloc( ti_reader->pool, index_size * sizeof(lcn_term_info_t) ),
               APR_ENOMEM );

        LCNPV( ti_reader->index_pointers = (apr_off_t*) apr_palloc( ti_reader->pool, index_size * sizeof(apr_off_t) ),
               APR_ENOMEM );

        ti_reader->index_size = index_size;

        do
        {
            if( APR_SUCCESS != ( s = lcn_term_enum_next( index_enum ) ) )
            {
                if( LCN_ERR_ITERATOR_NO_NEXT == s )
                {
                    break;
                }
                LCNCE( s );
            }

            LCNCE( lcn_term_create( ti_reader->index_terms + i,
                                    index_enum->term->field,
                                    index_enum->term->text,
                                    LCN_TERM_TEXT_COPY,
                                    ti_reader->pool ) );

            *(ti_reader->index_infos + i) = *(index_enum->term_info);

            (ti_reader->index_pointers)[i]  = index_enum->index_pointer;

            i++;
        }
        while(1);

        if ( LCN_ERR_ITERATOR_NO_NEXT == s )
        {
            s = APR_SUCCESS;
        }

        s = lcn_term_enum_close( index_enum );
    }
    while(0);

    if ( NULL != pool )
    {
        apr_pool_destroy( pool );
    }

    return s;
}


apr_status_t
lcn_term_infos_reader_get_by_term( lcn_term_infos_reader_t *ti_reader,
                                   lcn_term_info_t **term_info,
                                   const lcn_term_t *term )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_term_enum_t *t_enum = ti_reader->orig_enum;

        LCNASSERT( ti_reader->size > 0, LCN_ERR_TERM_INFOS_READER_EMPTY );
        LCNCE( lcn_term_infos_reader_ensure_index_is_read( ti_reader ) );

        /* optimize sequential access: first try scanning cached enum w/o seeking */

        if ( lcn_term_field( t_enum->term ) != NULL &&   /* term is at or past current */
             ( ( lcn_term_field( t_enum->prev ) != NULL && lcn_term_compare( term, t_enum->prev ) > 0 ) ||
               lcn_term_compare( term, t_enum->term ) >= 0 ))
        {
            apr_off_t enum_offset = ( t_enum->position / t_enum->index_interval ) + 1;

            /* @todo: check whether it is legal to be enum_offset > index_size */
            /* this occurs under some conditions                               */
            /* build a test case for this                                      */

            if ( ti_reader->index_size <= enum_offset ||  /* but before end of block */
                 lcn_term_compare( term, *(ti_reader->index_terms + enum_offset) ) < 0 )
            {
                /* no need to seek */
                s = lcn_term_infos_reader_scan_enum( ti_reader, term_info, term );
                break;
            }
        }

        {
            /*  random-access: must seek */
            apr_off_t offset = lcn_term_infos_reader_get_index_offset( ti_reader, term );

            LCNCE( lcn_term_infos_reader_seek_enum( ti_reader, offset ));

            /* don't log this, as we use status value as a warning here */

            if (( s = lcn_term_infos_reader_scan_enum( ti_reader, term_info, term )))
            {
                break;
            }
        }
    }
    while(0);

    return s;
}

apr_status_t
lcn_term_infos_reader_terms( lcn_term_infos_reader_t *ti_reader,
                             lcn_term_enum_t **term_enum,
                             apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        /* if not at start, reset to start */

        if ( ti_reader->orig_enum->position != -1)
        {
            LCNCE( lcn_term_infos_reader_seek_enum( ti_reader, 0) );
        }

        LCNCE( lcn_term_enum_clone( ti_reader->orig_enum, term_enum, pool ) );
    }
    while(0);

    return s;
}

/**
 * Enumeration of terms starting at or following given term.
 * Enumeration may not start with the given term, the
 * first element of the Enumeration is the smallest term
 * greater then the given.
 *
 * @see get_by_term in TermInfosReader
 */
apr_status_t
lcn_term_infos_reader_terms_from( lcn_term_infos_reader_t *ti_reader,
                                  lcn_term_enum_t **term_enum,
                                  lcn_term_t *term,
                                  apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_term_info_t *term_info;
        s = lcn_term_infos_reader_get_by_term( ti_reader, &term_info, term );

        if ( APR_SUCCESS == s ||
             LCN_ERR_SCAN_ENUM_NO_MATCH == s ||
             LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s )
        {
            apr_status_t stat =  lcn_term_enum_clone( ti_reader->orig_enum, term_enum, pool );

            if ( APR_SUCCESS != stat )
            {
                s = stat;
            }
        }
    }
    while(0);

    return s;
}


apr_status_t
lcn_term_infos_reader_create( lcn_term_infos_reader_t **term_infos_reader,
                              lcn_directory_t *dir,
                              const char *segment,
                              lcn_field_infos_t *field_infos,
                              apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        char *file_name;
        lcn_index_input_t *istream;

        LCNPV( *term_infos_reader = (lcn_term_infos_reader_t*) apr_pcalloc( pool, sizeof( lcn_term_infos_reader_t )),
               APR_ENOMEM );

        LCNPV( (*term_infos_reader)->segment = apr_pstrdup( pool, segment ), APR_ENOMEM );

        (*term_infos_reader)->pool = pool;
        (*term_infos_reader)->dir = dir;
        (*term_infos_reader)->field_infos = field_infos;

        LCNPV( file_name = apr_pstrcat( pool, segment, ".tis", NULL ), APR_ENOMEM );

        /* suppress regular error message here, as it is possible in a correct index  */
        /* it happens in indexes containing no indexed fields, as they occur in tests */
        {
            lcn_bool_t exists;
            LCNCE( lcn_directory_file_exists ( dir, file_name, &exists ));

            if ( exists )
            {
                LCNCE( lcn_directory_open_input( dir, &istream, file_name, pool ));
            }
            else
            {
                s = LCN_ERR_TERM_INFOS_READER_NO_TIS_FILE;
                break;
            }
        }

        LCNCE( lcn_segment_term_enum_create( &((*term_infos_reader)->orig_enum), istream, field_infos, LCN_FALSE, pool ));

        (*term_infos_reader)->size = lcn_term_enum_size( (*term_infos_reader)->orig_enum );
    }
    while(0);

    return s;
}


static apr_status_t
lcn_term_infos_reader_scan_enum_to_pos ( lcn_term_infos_reader_t* ti_reader,
                                         lcn_term_t **term,
                                         apr_off_t pos )
{
    apr_status_t s = APR_SUCCESS;

    lcn_term_enum_t *term_enum = ti_reader->orig_enum;

    while( term_enum->position < pos )
    {
        s = lcn_term_enum_next( term_enum );

        if ( LCN_ERR_ITERATOR_NO_NEXT == s )
        {
            return LCN_ERR_TERM_INFOS_READER_NO_TERM_AT_POS;
        }
        else if ( APR_SUCCESS != s )
        {
            break;
        }
    }

    if ( APR_SUCCESS == s )
    {
        *term = term_enum->term;
    }

    return s;
}

apr_status_t
lcn_term_infos_reader_get_term_at_pos( lcn_term_infos_reader_t *ti_reader,
                                       lcn_term_t **term,
                                       apr_off_t pos )
{
    apr_status_t s = APR_SUCCESS;

    lcn_term_enum_t *term_enum = ti_reader->orig_enum;

    if ( ti_reader->size == 0 )
    {
        return LCN_ERR_TERM_INFOS_READER_NO_TERM_AT_POS;
    }

    /* term_enum == NULL means that therm_infos_reader was */
    /* not correctly initialized. then the error should be */
    /* cought there. see lcn_term_infos_reader_create      */

    /* instead of term_enum->term != NULL we use           */
    /* term_enum->term->field != NULL to avoid allocs      */

    do
    {
        if ( ! (term_enum->term->field != NULL &&
                pos >= term_enum->position     &&
                pos < ( term_enum->position + term_enum->index_interval )))
        {
            /* must seek */

            LCNCE( lcn_term_infos_reader_seek_enum( ti_reader, term_enum->position / term_enum->index_interval ));
        }

        LCNCE( lcn_term_infos_reader_scan_enum_to_pos ( ti_reader, term, pos ) );
    }
    while(0);

    return s;
}

#if 0
static apr_status_t
lcn_term_infos_reader_get_position( lcn_term_infos_reader_t *ti_reader,
                                    apr_off_t *position,
                                    const lcn_term_t *term )
{
    apr_status_t s = APR_SUCCESS;

    if ( ti_reader->size == 0 )
    {
        return LCN_ERR_TERM_INFOS_READER_NO_TERM_AT_POS;
    }

    do
    {
        apr_off_t index_offset;
        lcn_term_enum_t *term_enum;

        LCNCE( lcn_term_infos_reader_ensure_index_is_read( ti_reader ));
        index_offset = lcn_term_infos_reader_get_index_offset( ti_reader, term );
        LCNCE( lcn_term_infos_reader_seek_enum( ti_reader, index_offset ));

        term_enum = ti_reader->orig_enum;

        while ( lcn_term_compare( term, term_enum->term ) > 0 )
        {
            s = lcn_term_enum_next( term_enum );

            if ( LCN_ERR_ITERATOR_NO_NEXT == s )
            {
                s = APR_SUCCESS;
                break;
            }
            else if ( APR_SUCCESS != s )
            {
                break;
            }
        }

        if( s )
        {
            break;
        }

        if ( lcn_term_compare( term, term_enum->term ) == 0 )
        {
            *position = term_enum->position;
        }
        else
        {
            s = LCN_ERR_TERM_INFOS_READER_NO_POS_FOR_TERM;
        }
    }
    while(0);

    return s;
}
#endif


apr_status_t
lcn_term_infos_reader_close( lcn_term_infos_reader_t *ti_reader )
{
    return lcn_term_enum_close( ti_reader->orig_enum );
}

unsigned int
lcn_term_infos_reader_size( lcn_term_infos_reader_t *ti_reader )
{
    return ti_reader->size;
}

unsigned int
lcn_term_infos_reader_skip_interval( lcn_term_infos_reader_t *ti_reader )
{
    return ti_reader->orig_enum->skip_interval;
}
