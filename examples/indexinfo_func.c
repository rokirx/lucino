#include <stdio.h>

#include "indexinfo_func.h"
#include "lcn_util.h"
#include "lcn_index.h"
#include "lcn_search.h"

/**
 * @brief Shows count of all docs within the index_reader
 *
 * @param index_reader
 *
 * @return s
 */
apr_status_t
lcn_index_reader_show_doc_count( lcn_index_reader_t* index_reader )
{
    int doc_all_count = lcn_index_reader_num_docs( index_reader );

    printf( "\n==== The whole amount of documents in this Index is: %d \n",
            doc_all_count );

    return APR_SUCCESS;
}

static int
field_info_sort_func( const void* a, const void* b )
{
    lcn_field_info_t **field_info_a = (lcn_field_info_t **) a;
    lcn_field_info_t **field_info_b = (lcn_field_info_t **) b;
    return strcmp( lcn_field_info_name( *field_info_a ),
                   lcn_field_info_name( *field_info_b ));
}

/**
 * @brief Shows all fields (with its settings) of the index_reader
 *
 * @param index_reader
 * @param pool
 *
 * @return s
 */
apr_status_t
lcn_index_reader_show_fields_overview( lcn_index_reader_t* index_reader,
                                       apr_pool_t* pool )
{
    int i, list_size;
    const char *name_str;
    lcn_list_t *field_info_list;
    lcn_field_info_t *field_info;
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int max_fname_len = strlen("  name ");

        LCNCE( lcn_list_create( &field_info_list, 10, pool ) );
        LCNCE( lcn_index_reader_get_field_infos( index_reader,
                                                 field_info_list,
                                                 0x0,
                                                 0x0 ) );

        list_size = lcn_list_size( field_info_list );

        for( i = 0; i < list_size; i++ )
        {
            field_info = lcn_list_get( field_info_list, i );
            name_str = lcn_field_info_name( field_info );

            if ( strlen( name_str) > max_fname_len )
            {
                max_fname_len = strlen( name_str );
            }
        }

        LCNCE( lcn_list_sort( field_info_list, field_info_sort_func ));

        printf( "\nOverview of fields and attributes\n");
        printf( "---------------------------------");
        printf( "\n\ncount  %-*s     indexed  omit_norms\n", max_fname_len+1, "  name ");

        for( i = 0; i < list_size; i++)
        {
            lcn_field_info_t *field_info = lcn_list_get( field_info_list, i );
            const char* name_str = lcn_field_info_name( field_info );
            lcn_bool_t is_indexed = lcn_field_info_is_indexed( field_info );
            lcn_bool_t omit_norms = lcn_field_info_omit_norms( field_info );
            //lcn_bool_t fixed_size = lcn_field_info_fixed_size( field_info );

            printf ( "\n%5d  %-*s:%10s%10s",
                     i,
                     max_fname_len+1,
                     name_str,
                     is_indexed ? "Y" : "N",
                     is_indexed ? (omit_norms ? "Y" : "N") : "-"
                     //fixed_size ? "Y" : "N"
                );
        }

        printf("\n");
    }
    while(0);

    return s;
}


/**
 * @brief Shows <number> of <term_text> from <field> starting at <term_text>
 *
 * "with it's <freq>" NOT STABLE JET shows -1 for now (at line 181)
 *
 * @param index_reader
 * @param field_name
 * @param term_text
 * @param number
 * @param pool @return s
 */
apr_status_t
lcn_index_reader_show_field_overview( lcn_index_reader_t* index_reader,
                                      const char* field_name,
                                      const char* term_text,
                                      const int  number,
                                      unsigned int term_freq_lo,
                                      unsigned int term_freq_hi,
                                      apr_pool_t* pool )
{

    apr_status_t s;
    int count, freq = 0;
    char *text;

    lcn_term_t *term;
    lcn_term_enum_t *term_enum;
    apr_status_t next_status = LCN_ERR_ITERATOR_NO_NEXT;
    lcn_term_docs_t *term_docs;
    apr_pool_t *cp = NULL;

    s = APR_SUCCESS;
    count = 0;

    do
    {
        LCNCE( lcn_term_create( &term, field_name, term_text, LCN_TERM_TEXT_COPY, pool ) );

        LCNCE( lcn_index_reader_terms_from( index_reader, &term_enum, term, pool ) );

        printf( "====  field_name: %s\n====  term      : %s\n====  number    : %d\n\n",
                field_name,
                term_text,
                number );

        printf( "\noverview of field '%s', starting at '%s'\n",
                field_name,
                term_text );

        printf( "------------------------------------------------\n\n");
        printf( "count\t\t   term\t\t  frequency\n" );
        printf( "-----\t\t   ----\t\t  ---------\n\n" );

        LCNCE( apr_pool_create( &cp, pool ));

        do
        {
            const lcn_term_t *t;

            next_status = lcn_term_enum_next( term_enum );
            t= lcn_term_enum_term( term_enum );
            text = (char*) lcn_term_text( t );

            LCNCE( lcn_index_reader_term_docs_from( index_reader, &term_docs, t, cp ) );

            if ( 0 != strcmp( field_name, lcn_term_field(t) ))
            {
                break;
            }

            freq = lcn_term_enum_doc_freq( term_enum );

            if ( ( 0 == term_freq_lo && 0 == term_freq_hi ) ||
                 ( term_freq_lo > term_freq_hi && term_freq_hi != 0 ) ||
                 ( term_freq_lo <= freq && freq <= term_freq_hi ) ||
                 ( term_freq_lo <= freq && term_freq_hi == 0 ) ||
                 ( term_freq_lo == 0 && freq <= term_freq_hi ))
            {
                count++;
                printf( " %4d %15s %25s \t%d\n", count, lcn_term_field(t), text, freq );
            }

            apr_pool_clear( cp );

            if ( APR_SUCCESS != next_status )
            {
                break;
            }

            if( ( count == number ) || ( count > number ) )
            {
                LCNCE( lcn_term_enum_close( term_enum ) );
                break;
            }
        }
        while( APR_SUCCESS == next_status );
    }
    while( 0 );

    if ( NULL == cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}


apr_status_t
lcn_do_query_search( lcn_index_reader_t *index_reader,
                     const char *qstring,
                     const char *output_fields,
                     unsigned int limit,
                     apr_pool_t *pool )
{
    apr_status_t s;
    apr_pool_t* cp = NULL;

    do
    {
        lcn_searcher_t* searcher;
        lcn_hits_t *hits;
        lcn_query_t *query;
        unsigned int i, max_fname_len;
        lcn_list_t* fields_list;

        LCNCE( lcn_index_searcher_create_by_reader( &searcher,
                                                    index_reader,
                                                    pool ));

        LCNCE( lcn_parse_query( &query, qstring, pool ));

        LCNCE( lcn_searcher_search( searcher,
                                    &hits,
                                    query,
                                    NULL,
                                    pool ));
        printf( "Die Suche nach '%s' ergab\n %d Treffer.\n\n",
                qstring, lcn_hits_length( hits ));
        {
            char *last;
            char *token = apr_strtok( apr_pstrdup( pool, output_fields ), ",", &last);
            max_fname_len = 0;

            LCNCE( lcn_list_create( &fields_list, 10, pool ));

            while( token )
            {
                LCNCE( lcn_list_add( fields_list, apr_pstrdup( pool, token )));
                if ( strlen(token) > max_fname_len )
                {
                    max_fname_len = strlen(token);
                }
                token = apr_strtok( NULL, ",", &last );
            }
        }

        if ( limit > lcn_hits_length( hits ))
        {
            limit = lcn_hits_length( hits );
        }

        for( i = 0; i < limit; i++ )
        {
            lcn_document_t* doc;
            char* field_text;
            unsigned int fcount;

            if ( NULL == cp )
            {
                LCNCE( apr_pool_create( &cp, pool ));
            }

            apr_pool_clear( cp );

            LCNCE( lcn_hits_doc( hits, &doc, i, cp ) );

            printf( "%d: \n", i );

            for( fcount = 0; fcount < lcn_list_size( fields_list ); fcount++ )
            {
                char *fname = (char*) lcn_list_get( fields_list, fcount );

                if ( 0 == strcmp( fname, "fsfgelt" ))
                {
                    unsigned int val;
                    LCNCE( lcn_document_get_int( doc, "fsfgelt", &val ));
                    printf( "%-*s: %u", max_fname_len+1, "fsfgelt", val );
                }
                else
                {
                    LCNCE( lcn_document_get( doc, &field_text, fname, cp ) );
                    printf( "%-*s: %s", max_fname_len+1, fname, field_text );
                }
                printf("\n");
            }
            printf("\n");
        }
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}
