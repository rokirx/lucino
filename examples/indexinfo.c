#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apr_getopt.h>
#include "lucene.h"
#include "lcn_index.h"
#include "indexinfo_func.h"

/* FILE* lcn_log_stream; */

#define LIST_FIELDS  (1)
#define TERM_ENUM    (2)
#define SEARCH_DOCS  (3)

static void
print_help(void)
{
    printf( "\nAufruf:\n\n"
            "indexinfo [options] index_dir\n\n"
            "Optionen:\n\n"
            "--help             : Gibt diese Hilfe aus\n"
            "--list-fields      : Listet die Felder des Index mit den\n"
            "                     vorhandenen Informationen auf\n"
            "--term-enum        : Listet die Terms des Index auf\n"
            "--field            : Gibt fuer --term-enum das aufzulistende Feld an\n"
            "--term             : Gibt fuer --term-enum den Term an, ab dem die\n"
            "                     die Auflistung starten soll\n"
            "--max-terms        : Gibt fuer --term-enum die maximale Anzahl der aufzulistenden\n"
            "                     Terms an\n"
            "--term-freq        : Gibt fuer --term-enum die minimale bzw. maximale Haeufigkeit der\n"
            "                     Terms an, die aufgelistet werden sollen. Beispiele:\n"
            "                       --term-freq=1       nur Terms mit der Haeufigkeit 1 ausgeben\n"
            "                       --term-freq=5:      nur Terms mit der Haeufigkeit 5 und groesser ausgeben\n"
            "                       --term-freq=:5      nur Terms mit der Haeufigkeit kleiner gleich 5 ausgeben\n"
            "                       --term-freq=5:10    nur Terms mit der Haeufigkeit 5 bis einschliesslich 10 ausgeben\n"
            "--query            : Query fuer die Suche im Index\n"
            "--output-fields    : Ein Stored-Field fuer die Darstellung der Suchergebnisse\n"
            "--limit            : Limitiert Ausgabe der Ergebnisse\n"
        );
}


/*main
 *
 *USE:
 *
 *indexinfo (no index_path then default ./test/test_index_1)
 *indexinfo [index_path]
 *indexinfo [index_path] [field_name] [term_text] (no number then default 15)
 *indexinfo [index_path] [field_name] [term_text] [number]
 *
 *@agc
 *@argv
 *
 *@return 0
*/
int main( int argc, char** argv )
{
    apr_status_t s;
    apr_pool_t *pool;

    static const apr_getopt_option_t long_options[] =
        {
            {"help",               1,     FALSE },
            {"list-fields",        2,     FALSE },
            {"field",              3,     TRUE  },
            {"term",               4,     TRUE  },
            {"term-enum",          5,     FALSE },
            {"max-terms",          6,     TRUE  },
            {"term-freq",          7,     TRUE  },
            {"query",              8,     TRUE  },
            {"output-fields",      9,     TRUE  },
            {"limit",             10,     TRUE  },
            {"dump",              12,     TRUE  },
            { NULL,                0,     0     }
    };


    do
    {
        char *index_path = NULL;
        const char *field = "";
        const char *term_text = "";
        int number = 15;
        apr_getopt_t* os;
        int optch;
        const char* optarg;
        lcn_index_reader_t *index_reader = NULL;
        int action = 0;
        unsigned int term_freq_lo = 0;
        unsigned int term_freq_hi = 0;
        char *query_string = 0;
        unsigned int limit = 15;
        char *output_fields = NULL;

        apr_initialize();
        LCNCE( apr_pool_create( &pool, NULL));
        lcn_atom_init( pool );
        lcn_log_stream = stderr;

        if ( argc < 2 )
        {
            print_help();
            exit(0);
        }

        LCNCE( apr_getopt_init( &os, pool, argc, (const char* const*)argv ) );

        while ( APR_SUCCESS == (s = apr_getopt_long( os, long_options, &optch, &optarg ) ))
        {
            switch( optch )
            {
            case 1:
                print_help();
                exit(0);
            case 2:
                action = LIST_FIELDS;
                break;
            case 3:
                field = apr_pstrdup( pool, (char*) optarg );
                break;
            case 4:
                term_text = apr_pstrdup( pool, (char *) optarg );
                break;
            case 5:
                action = TERM_ENUM;
                break;
            case 6:
                number = atoi( (char*) optarg );
                break;
            case 7:
                {
                    char *tf = apr_pstrdup( pool, (char*) optarg );
                    char *col;

                    if ( ! (col = strchr( tf, ':') ))
                    {
                        term_freq_lo = term_freq_hi = atoi( tf );
                    }
                    else
                    {
                        *col = '\0';
                        term_freq_lo = atoi( tf );

                        if ( *(col+1) )
                        {
                            term_freq_hi = atoi( col+1);
                        }
                    }
                }
                break;
            case 8:
                query_string = apr_pstrdup( pool, (char*) optarg);
                action = SEARCH_DOCS;
                break;
            case 9:
                output_fields = apr_pstrdup( pool, (char*) optarg );
                break;
            case 10:
                limit = atoi( (char*) optarg);
                break;
            }
        }

        if ( os->ind < argc )
        {
            index_path = apr_pstrdup( pool, argv[os->ind] );
        }
        else
        {
            fprintf( stderr, "Es wurde kein Ausgabepfad angegeben... Abbruch!\n" );
            s = -1;
            break;
        }

        LCNCE( ( lcn_index_reader_create_by_path( &index_reader, index_path, pool ) ) );

        switch( action )
        {
        case LIST_FIELDS:
            LCNCE( lcn_index_reader_show_doc_count( index_reader ) );
            LCNCE( lcn_index_reader_show_fields_overview( index_reader, pool ) );
            break;
        case TERM_ENUM:
            LCNCE( lcn_index_reader_show_field_overview( index_reader,
                                                         field,
                                                         term_text,
                                                         number,
                                                         term_freq_lo,
                                                         term_freq_hi,
                                                         pool ) );
            break;
        case SEARCH_DOCS:
            LCNCE( lcn_do_query_search( index_reader,
                                        query_string,
                                        output_fields,
                                        limit,
                                        pool ));
            break;
        default: break;
        }
    }
    while(0);

    apr_pool_destroy( pool );
    atexit( apr_terminate );

    return 0;
}
