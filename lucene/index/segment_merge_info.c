#include "lucene.h"
#include "lcn_util.h"
#include "lcn_segment_merge_info.h"

apr_status_t
lcn_segment_merge_info_get_doc_map( lcn_segment_merge_info_t *smi,
                                    lcn_int_array_t **doc_map )
{
    apr_status_t s = APR_SUCCESS;

    /* build array which maps document numbers around deletions  */

    if ( NULL == smi->doc_map &&
         lcn_index_reader_has_deletions( smi->reader ) )
    {
        unsigned int i;
        unsigned int j = 0;
        unsigned int max_doc = lcn_index_reader_max_doc( smi->reader );
        LCNCR( lcn_int_array_create( &smi->doc_map, max_doc, smi->pool ));

        for( i = 0; i < max_doc; i++ )
        {
            if ( lcn_index_reader_is_deleted( smi->reader, i ) )
            {
                smi->doc_map->arr[ i ] = -1;
            }
            else
            {
                smi->doc_map->arr[ i ] = j++;
            }
        }
    }

    *doc_map = smi->doc_map;

    return s;
}

apr_status_t
lcn_segment_merge_info_get_positions( lcn_segment_merge_info_t *smi,
                                      lcn_term_docs_t **postings )
{
    apr_status_t s = APR_SUCCESS;

    if ( NULL == smi->postings )
    {
        LCNCR( lcn_index_reader_term_positions( smi->reader, &(smi->postings), smi->pool ) );
    }

    *postings = smi->postings;

    return s;
}


apr_status_t
lcn_segment_merge_info_create( lcn_segment_merge_info_t **segment_merge_info,
                               int base,
                               lcn_term_enum_t *term_enum,
                               lcn_index_reader_t *index_reader,
                               apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *segment_merge_info = (lcn_segment_merge_info_t *)
               apr_pcalloc( pool, sizeof(lcn_segment_merge_info_t) ), APR_ENOMEM );

        (*segment_merge_info)->base      = base;
        (*segment_merge_info)->reader    = index_reader;
        (*segment_merge_info)->term_enum = term_enum;
        (*segment_merge_info)->term_enum = term_enum;
        (*segment_merge_info)->term      = lcn_term_enum_term( term_enum );
        (*segment_merge_info)->pool      = pool;
    }
    while(0);

    return s;

    return APR_SUCCESS;
}

apr_status_t
lcn_segment_merge_info_close( lcn_segment_merge_info_t *segment_merge_info )
{
    apr_status_t s = APR_SUCCESS;
    apr_status_t tmp;

    if (( tmp = lcn_term_enum_close( segment_merge_info->term_enum )))
    {
        s = tmp;
    }

    if ( NULL != segment_merge_info->postings )
    {
        //lcn_term_docs_close( postings )
    }

    return s;
}


apr_status_t
lcn_segment_merge_info_next( lcn_segment_merge_info_t *smi )
{
    apr_status_t s = lcn_term_enum_next( smi->term_enum );
    // const lcn_term_t *t = lcn_term_enum_term( smi->term_enum );
    // fprintf(stderr, "smi next %s:%s\n", lcn_term_field( t ), lcn_term_text( t ));
    smi->term = ( APR_SUCCESS == s ? lcn_term_enum_term( smi->term_enum ) : NULL );
    return s;
}
