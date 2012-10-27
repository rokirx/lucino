#include "lucene.h"
#include "test_all.h"
#include "term_infos_reader.h"

char *test_data[374][2] = {
    { "id", "KW0" },
    { "id", "KW1" },
    { "id", "KW10" },
    { "id", "KW11" },
    { "id", "KW12" },
    { "id", "KW13" },
    { "id", "KW14" },
    { "id", "KW15" },
    { "id", "KW16" },
    { "id", "KW17" },
    { "id", "KW18" },
    { "id", "KW19" },
    { "id", "KW2"  },
    { "id", "KW20" },
    { "id", "KW21" },
    { "id", "KW22" },
    { "id", "KW23" },
    { "id", "KW24" },
    { "id", "KW25" },
    { "id", "KW26" },
    { "id", "KW27" },
    { "id", "KW28" },
    { "id", "KW29" },
    { "id", "KW3"  },
    { "id", "KW30" },
    { "id", "KW31" },
    { "id", "KW32" },
    { "id", "KW33" },
    { "id", "KW34" },
    { "id", "KW35" },
    { "id", "KW36" },
    { "id", "KW37" },
    { "id", "KW38" },
    { "id", "KW39" },
    { "id", "KW4"  },
    { "id", "KW40" },
    { "id", "KW41" },
    { "id", "KW42" },
    { "id", "KW43" },
    { "id", "KW44" },
    { "id", "KW45" },
    { "id", "KW46" },
    { "id", "KW47" },
    { "id", "KW48" },
    { "id", "KW49" },
    { "id", "KW5" },
    { "id", "KW50" },
    { "id", "KW51" },
    { "id", "KW52" },
    { "id", "KW53" },
    { "id", "KW54" },
    { "id", "KW55" },
    { "id", "KW56" },
    { "id", "KW57" },
    { "id", "KW58" },
    { "id", "KW59" },
    { "id", "KW6" },
    { "id", "KW60" },
    { "id", "KW61" },
    { "id", "KW62" },
    { "id", "KW63" },
    { "id", "KW64" },
    { "id", "KW65" },
    { "id", "KW66" },
    { "id", "KW67" },
    { "id", "KW68" },
    { "id", "KW69" },
    { "id", "KW7"  },
    { "id", "KW70" },
    { "id", "KW71" },
    { "id", "KW72" },
    { "id", "KW73" },
    { "id", "KW74" },
    { "id", "KW75" },
    { "id", "KW76" },
    { "id", "KW8"  },
    { "id", "KW9"  },
    { "text", "a"  },
    { "text", "abstract" },
    { "text", "add" },
    { "text", "adddocument" },
    { "text", "added" },
    { "text", "addindexes" },
    { "text", "adjusted" },
    { "text", "advantage" },
    { "text", "after" },
    { "text", "all" },
    { "text", "also" },
    { "text", "alter" },
    { "text", "altering" },
    { "text", "amok" },
    { "text", "an" },
    { "text", "analyzer" },
    { "text", "analyzers" },
    { "text", "and" },
    { "text", "arbitrary" },
    { "text", "are" },
    { "text", "as" },
    { "text", "be" },
    { "text", "behaviour" },
    { "text", "better" },
    { "text", "between" },
    { "text", "boolean" },
    { "text", "booleanquery" },
    { "text", "boosting" },
    { "text", "both" },
    { "text", "bschneeman" },
    { "text", "bug" },
    { "text", "bugs" },
    { "text", "build" },
    { "text", "but" },
    { "text", "by" },
    { "text", "c" },
    { "text", "cachingwrapperfilter" },
    { "text", "can" },
    { "text", "cases" },
    { "text", "catch" },
    { "text", "change" },
    { "text", "changed" },
    { "text", "characters" },
    { "text", "chinese" },
    { "text", "christoph" },
    { "text", "cjk" },
    { "text", "classes" },
    { "text", "clauses" },
    { "text", "cleaned" },
    { "text", "code" },
    { "text", "combined" },
    { "text", "comments" },
    { "text", "compiling" },
    { "text", "compound" },
    { "text", "confusing" },
    { "text", "contiguous" },
    { "text", "controlling" },
    { "text", "correctly" },
    { "text", "created" },
    { "text", "cutting" },
    { "text", "daniel" },
    { "text", "date" },
    { "text", "dealing" },
    { "text", "default" },
    { "text", "defined" },
    { "text", "demo" },
    { "text", "different" },
    { "text", "distinguish" },
    { "text", "dmitry" },
    { "text", "doccount" },
    { "text", "document" },
    { "text", "documents" },
    { "text", "doug" },
    { "text", "e" },
    { "text", "each" },
    { "text", "easily" },
    { "text", "eliminate" },
    { "text", "enhancements" },
    { "text", "enumeration" },
    { "text", "erik" },
    { "text", "exp" },
    { "text", "explain" },
    { "text", "field" },
    { "text", "fields" },
    { "text", "file" },
    { "text", "files" },
    { "text", "final" },
    { "text", "fix" },
    { "text", "fixed" },
    { "text", "fixes" },
    { "text", "folks" },
    { "text", "for" },
    { "text", "format" },
    { "text", "frequences" },
    { "text", "from" },
    { "text", "fuzzy" },
    { "text", "fuzzytermenum" },
    { "text", "g" },
    { "text", "generates" },
    { "text", "getfieldnames" },
    { "text", "getproperty" },
    { "text", "goller" },
    { "text", "had" },
    { "text", "handling" },
    { "text", "hang" },
    { "text", "hatcher" },
    { "text", "hfs" },
    { "text", "html" },
    { "text", "id" },
    { "text", "ideogram" },
    { "text", "ideograms" },
    { "text", "implementation" },
    { "text", "in" },
    { "text", "incorrect" },
    { "text", "index" },
    { "text", "indexes" },
    { "text", "indexing" },
    { "text", "indexreader" },
    { "text", "indexsearcher" },
    { "text", "indexwriter" },
    { "text", "insert" },
    { "text", "instead" },
    { "text", "introduced" },
    { "text", "io" },
    { "text", "is" },
    { "text", "it" },
    { "text", "its" },
    { "text", "japanese" },
    { "text", "java" },
    { "text", "javascript" },
    { "text", "julien" },
    { "text", "kirchgessner" },
    { "text", "korean" },
    { "text", "length" },
    { "text", "lengthnorm" },
    { "text", "limit" },
    { "text", "locale" },
    { "text", "lock" },
    { "text", "locking" },
    { "text", "log" },
    { "text", "long" },
    { "text", "longer" },
    { "text", "low" },
    { "text", "lucene" },
    { "text", "macos" },
    { "text", "maxfieldlength" },
    { "text", "may" },
    { "text", "memory" },
    { "text", "merging" },
    { "text", "method" },
    { "text", "methods" },
    { "text", "minmergedocs" },
    { "text", "minor" },
    { "text", "modified" },
    { "text", "more" },
    { "text", "most" },
    { "text", "multiindexsearcher" },
    { "text", "multiple" },
    { "text", "naber" },
    { "text", "needed" },
    { "text", "new" },
    { "text", "nioche" },
    { "text", "no" },
    { "text", "normalization" },
    { "text", "not" },
    { "text", "now" },
    { "text", "nullpointerexception" },
    { "text", "number" },
    { "text", "object" },
    { "text", "of" },
    { "text", "on" },
    { "text", "one" },
    { "text", "only" },
    { "text", "open" },
    { "text", "otis" },
    { "text", "outofmemoryexceptions" },
    { "text", "output" },
    { "text", "outside" },
    { "text", "package" },
    { "text", "parseexception" },
    { "text", "parser" },
    { "text", "parsing" },
    { "text", "patch" },
    { "text", "perfieldanalyzerwrapper" },
    { "text", "permits" },
    { "text", "permitted" },
    { "text", "permitting" },
    { "text", "phrase" },
    { "text", "phrasequery" },
    { "text", "place" },
    { "text", "position" },
    { "text", "prefix" },
    { "text", "present" },
    { "text", "previously" },
    { "text", "priorityqueue" },
    { "text", "private" },
    { "text", "problem" },
    { "text", "problems" },
    { "text", "queries" },
    { "text", "query" },
    { "text", "queryparser" },
    { "text", "raised" },
    { "text", "range" },
    { "text", "rc" },
    { "text", "read" },
    { "text", "replace" },
    { "text", "resolution" },
    { "text", "resolves" },
    { "text", "returned" },
    { "text", "rewriting" },
    { "text", "run" },
    { "text", "running" },
    { "text", "s" },
    { "text", "same" },
    { "text", "score" },
    { "text", "searching" },
    { "text", "see" },
    { "text", "segmentreader" },
    { "text", "segments" },
    { "text", "segmentsreader" },
    { "text", "separate" },
    { "text", "sequences" },
    { "text", "serebrennikov" },
    { "text", "setnorm" },
    { "text", "setpositionincrement" },
    { "text", "setting" },
    { "text", "should" },
    { "text", "similarity" },
    { "text", "single" },
    { "text", "skip" },
    { "text", "slightly" },
    { "text", "so" },
    { "text", "some" },
    { "text", "specific" },
    { "text", "speed" },
    { "text", "standardtokenizer" },
    { "text", "starting" },
    { "text", "stop" },
    { "text", "stored" },
    { "text", "stuff" },
    { "text", "subclassed" },
    { "text", "support" },
    { "text", "system" },
    { "text", "systems" },
    { "text", "t" },
    { "text", "take" },
    { "text", "term" },
    { "text", "termenum" },
    { "text", "terms" },
    { "text", "testrussianstem" },
    { "text", "than" },
    { "text", "that" },
    { "text", "the" },
    { "text", "them" },
    { "text", "these" },
    { "text", "things" },
    { "text", "this" },
    { "text", "throw" },
    { "text", "thrown" },
    { "text", "timestamp" },
    { "text", "timestamps" },
    { "text", "titles" },
    { "text", "tmpdir" },
    { "text", "to" },
    { "text", "token" },
    { "text", "tokenmgrerror" },
    { "text", "tokens" },
    { "text", "toomanyclauses" },
    { "text", "total" },
    { "text", "under" },
    { "text", "up" },
    { "text", "usage" },
    { "text", "use" },
    { "text", "used" },
    { "text", "useful" },
    { "text", "users" },
    { "text", "using" },
    { "text", "v" },
    { "text", "value" },
    { "text", "version" },
    { "text", "versus" },
    { "text", "very" },
    { "text", "via" },
    { "text", "was" },
    { "text", "way" },
    { "text", "were" },
    { "text", "when" },
    { "text", "where" },
    { "text", "which" },
    { "text", "whitespace" },
    { "text", "wildcard" },
    { "text", "with" },
    { "text", "without" },
    { "text", "work" },
    { "text", "write" },
    { "text", "writer" },
    { "text", "x" }
};

static void
test_term_infos_reader(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &dir, "test_index_1", LCN_FALSE, pool ) );

    {
        apr_pool_t *reader_pool;
        lcn_term_infos_reader_t *ti_reader;
        lcn_field_infos_t *field_infos;

        LCN_TEST( apr_pool_create( &reader_pool, pool ) );

        LCN_TEST( lcn_field_infos_create_from_dir( &field_infos, dir, "_2c", reader_pool ))
        LCN_TEST( lcn_term_infos_reader_create( &ti_reader, dir, "_2c", field_infos, reader_pool ) );

        {
            apr_pool_t *te_pool;
            lcn_term_enum_t *term_enum;
            apr_status_t next_status;
            int i  = 0;

            LCN_TEST( apr_pool_create( &te_pool, pool ) );
            LCN_TEST( lcn_term_infos_reader_terms( ti_reader, &term_enum, te_pool ) );

            do
            {
                next_status = lcn_term_enum_next( term_enum );

                if ( APR_SUCCESS != next_status )
                {
                    break;
                }

                CuAssertStrEquals(tc, test_data[i][0], lcn_term_field( lcn_term_enum_term( term_enum )) );
                CuAssertStrEquals(tc, test_data[i][1], lcn_term_text( lcn_term_enum_term( term_enum )) );

                i++;
            }
            while( APR_SUCCESS == next_status );

            CuAssertIntEquals( tc, 374, i );

            LCN_TEST( lcn_term_enum_close( term_enum ) );
            apr_pool_destroy( te_pool );
        }

        {
            apr_pool_t *te_pool;
            lcn_term_enum_t *term_enum;
            apr_status_t next_status;
            int i  = 0;

            LCN_TEST( apr_pool_create( &te_pool, pool ) );
            LCN_TEST( lcn_term_infos_reader_terms( ti_reader, &term_enum, te_pool ) );

            do
            {
                next_status = lcn_term_enum_next( term_enum );

                if ( APR_SUCCESS != next_status )
                {
                    break;
                }

                CuAssertStrEquals(tc, test_data[i][0], lcn_term_field( lcn_term_enum_term( term_enum )) );
                CuAssertStrEquals(tc, test_data[i][1], lcn_term_text( lcn_term_enum_term( term_enum )) );

                i++;
            }
            while( APR_SUCCESS == next_status );

            CuAssertIntEquals( tc, 374, i );

            LCN_TEST( lcn_term_enum_close( term_enum ) );
            apr_pool_destroy( te_pool );
        }

        {
            apr_pool_t *te_pool;
            lcn_term_enum_t *term_enum;
            apr_status_t next_status;
            lcn_term_t *term;
            lcn_term_info_t *term_info;
            int i  = 0;

            LCN_TEST( apr_pool_create( &te_pool, pool ) );

            LCN_TEST( lcn_term_infos_reader_get_term_at_pos( ti_reader, &term, 100 ));
            CuAssertStrEquals( tc, "better", term->text );
            CuAssertStrEquals( tc, "text", term->field );

            LCN_TEST( lcn_term_create( &term, "id", "KW47", 0, te_pool ) );

            LCN_TEST( lcn_term_infos_reader_get_by_term( ti_reader, &term_info, term ) );

            CuAssertIntEquals(tc, 1, term_info->doc_freq );
            CuAssertIntEquals(tc, 42, term_info->freq_pointer );
            CuAssertIntEquals(tc, 42, term_info->prox_pointer );

            LCN_TEST( lcn_term_infos_reader_terms_from( ti_reader, &term_enum, term, te_pool ) );

            i = 43;

            do
            {
                next_status = lcn_term_enum_next( term_enum );

                if ( APR_SUCCESS != next_status )
                {
                    break;
                }

                CuAssertStrEquals(tc, test_data[i][0], lcn_term_field( lcn_term_enum_term( term_enum )) );
                CuAssertStrEquals(tc, test_data[i][1], lcn_term_text( lcn_term_enum_term( term_enum )) );

                i++;
            }
            while( APR_SUCCESS == next_status );

            CuAssertIntEquals( tc, 374, i );

            LCN_TEST( lcn_term_enum_close( term_enum ) );
            apr_pool_destroy( te_pool );
        }

        LCN_TEST( lcn_term_infos_reader_close( ti_reader ));
    }
}


CuSuite *make_term_infos_reader_suite (void)
{	
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST(s,test_term_infos_reader );
    return s;
}
