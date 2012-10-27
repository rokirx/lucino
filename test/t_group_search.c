#include "lcn_search.h"
#include "test_all.h"
#include "lcn_index.h"


static char *test_index_name;

static void setup()
{
    test_index_name = "test_index_2";
}

/*

Hits, as they come without grouping

0 - 50) ->(1)(1.000000)  4. Minor enhancements to FuzzyTermEnum.
1 - 248) ->(1)(1.000000)  9. Upgraded to JUnit 3.7. (otis)
2 - 278) ->(1)(1.000000)  - added sources to distribution
3 - 97) ->(1)(0.883883)  7. Modified QueryParser to make it possible to programmatically specify the
4 - 207) ->(1)(0.883883)  the BUILD.txt document to describe how to override the
5 - 53) ->(1)(0.875000)  and MultiIndexSearcher to use it.
6 - 320) ->(1)(0.875000)  StandardTokenizer is to use StandardAnalyzer.
7 - 89) ->(1)(0.866025)  4. Added id method to Hits to be able to access the index global id.
8 - 41) ->(1)(0.750000)  1. Added getFieldNames(boolean) to IndexReader, SegmentReader, and
9 - 71) ->(1)(0.750000)  new method, IndexWriter.addIndexes(IndexReader[]), to take
10 - 185) ->(1)(0.750000)  (TermQuery, PhraseQuery and BooleanQuery), permitting access to
11 - 191) ->(1)(0.750000)  arguments, for easy FSDirectory to RAMDirectory conversion.
12 - 227) ->(1)(0.750000)  12. Add escape character to query parser.
13 - 235) ->(1)(0.750000)  Add XML Document #3 implementation to Document Section.
14 - 252) ->(1)(0.750000)  a RAMDirectory index to an FSDirectory.
15 - 263) ->(1)(0.750000)  7. Added 'contributions' section to website & docs. (carlson)
16 - 268) ->(1)(0.750000)  ability to reuse TermDocs objects.  (cutting)
17 - 293) ->(1)(0.750000)  - license switched from LGPL to Apache
18 - 73) ->(1)(0.707107)  12. Added a limit to the number of clauses which may be added to a
19 - 94) ->(1)(0.707107)  6. Added the ability to retrieve HTML documents' META tag values to
20 - 197) ->(1)(0.707107)  25. Refactored QueryParser to make it easier for people to extend it.
21 - 3) ->(1)(0.625000)  1. Added catch of BooleanQuery$TooManyClauses in QueryParser to
22 - 6) ->(1)(0.625000)  3. Added a new method IndexReader.setNorm(), that permits one to
23 - 21) ->(1)(0.625000)  1. Added minMergeDocs in IndexWriter.  This can be raised to speed
24 - 43) ->(1)(0.625000)  2. Changed file locking to place lock files in
25 - 46) ->(1)(0.625000)  lock indexes which are read-only to them.
26 - 48) ->(1)(0.625000)  permitting one to easily use different analyzers for different
27 - 57) ->(1)(0.625000)  7. Fixed SegmentsReader to eliminate the confusing and slightly different
28 - 86) ->(1)(0.625000)  3. Added the ability to disable lock creation by using disableLuceneLocks
29 - 92) ->(1)(0.625000)  5. Added support for new range query syntax to QueryParser.jj.
30 - 109) ->(1)(0.625000)  11. Changed the German stemming algorithm to ignore case while
31 - 131) ->(1)(0.625000)  used to boost scores of matches on that token.  (cutting)
32 - 140) ->(1)(0.625000)  need to be reconstructed once per day. (cutting)
33 - 142) ->(1)(0.625000)  analyzer used when adding documents to this index. (cutting)
34 - 150) ->(1)(0.625000)  20. Added getFieldNames() to IndexReader and Segment(s)Reader classes.
35 - 164) ->(1)(0.625000)  like "(+foo +bar)^2 +baz" is now supported and equivalent to
36 - 176) ->(1)(0.625000)  entire index.  This is intended to be used in developing
37 - 180) ->(1)(0.625000)  possible for someone to write a Scorer implementation that is
38 - 184) ->(1)(0.625000)  g. Added public accessors to the primitive query classes
39 - 198) ->(1)(0.625000)  Added the ability to automatically lower-case Wildcard terms in
40 - 202) ->(1)(0.625000)  1. Changed QueryParser.jj to have "?" be a special character which
41 - 206) ->(1)(0.625000)  1. Renamed build.properties to default.properties and updated
42 - 208) ->(1)(0.625000)  default.property settings without having to edit the file. This
43 - 209) ->(1)(0.625000)  brings the build process closer to Scarab's build process.
44 - 218) ->(1)(0.625000)  7. Added fix to avoid NullPointerException in results.jsp
45 - 236) ->(1)(0.625000)  Also added Term Highlighting to Misc Section. (carlson)
46 - 239) ->(1)(0.625000)  3. Changed document deletion code to obtain the index write lock,
47 - 245) ->(1)(0.625000)  7. Changed Term and Query to implement Serializable.  (scottganyo)
48 - 246) ->(1)(0.625000)  8. Fixed to never delete indexes added with IndexWriter.addIndexes().
49 - 251) ->(1)(0.625000)  empty index failed.  This was encountered using addIndexes to copy
50 - 292) ->(1)(0.625000)  - packages renamed from com.lucene to org.apache.lucene
51 - 313) ->(1)(0.625000)  new tokenizer uses a regular-expression grammar to identify more
52 - 319) ->(1)(0.625000)  The easiest way for applications to start using
53 - 13) ->(1)(0.500000)  these cases, the total number of tokens is a better value to use
54 - 27) ->(1)(0.500000)  4. Fix bug #19253, in demo HTML parser, add whitespace as needed to
55 - 45) ->(1)(0.500000)  permitted to write files.  This way folks can open and correctly
56 - 66) ->(1)(0.500000)  10. Added Locale setting to QueryParser, for use by date range parsing.
57 - 116) ->(1)(0.500000)  should be re-created from scratch in order for search scores to
58 - 127) ->(1)(0.500000)  and exact phrase query.  This makes it possible, e.g., to build
59 - 133) ->(1)(0.500000)  results to only match those which also match a provided query.
60 - 137) ->(1)(0.500000)  date field to implement date filtering.  One could re-use a
61 - 149) ->(1)(0.500000)  19. Fixed return of Hits.id() from float to int. (Terry Steichen via Peter).
62 - 157) ->(1)(0.500000)  it possible, e.g., to reuse the same query instance with
63 - 167) ->(1)(0.500000)  query to re-write itself as an alternate, more primitive query.
64 - 182) ->(1)(0.500000)  fairly advanced programming, and I don't expect anyone to do
65 - 203) ->(1)(0.500000)  allowed it to be used as a wildcard term. Updated TestWildcard
66 - 220) ->(1)(0.500000)  8. Changed Wildcard search to find 0 or more char instead of 1 or more
67 - 250) ->(1)(0.500000)  1. IndexWriter: fixed a bug where adding an optimized index to an
68 - 257) ->(1)(0.500000)  4. Fix query parser so that PrefixQuery is used in preference to
69 - 265) ->(1)(0.500000)  Folks must now download this separately from metamata in order to

Hits, as they should come

*/
int test_search_data[][7] =
{
    { 0, 3, 50, 53, 57, -1, -1 },
/*
0 - 50) ->(1)(1.000000)  4. Minor enhancements to FuzzyTermEnum.
  - 53) ->(1)(0.875000)  and MultiIndexSearcher to use it.
  - 57) ->(1)(0.625000)  7. Fixed SegmentsReader to eliminate the confusing and slightly different
*/
    { 1, 3, 248, 245, 246, -1, -1 },
/*
1 - 248) ->(1)(1.000000)  9. Upgraded to JUnit 3.7. (otis)
  - 245) ->(1)(0.625000)  7. Changed Term and Query to implement Serializable.  (scottganyo)
  - 246) ->(1)(0.625000)  8. Fixed to never delete indexes added with IndexWriter.addIndexes().
*/
    { 2, 1, 278, -1, -1, -1, -1},
/*
2 - 278) ->(1)(1.000000)  - added sources to distribution
*/
    { 3, 3, 97, 94, 92, -1, -1},
/*
3 - 97) ->(1)(0.883883)  7. Modified QueryParser to make it possible to programmatically specify the
  - 94) ->(1)(0.707107)  6. Added the ability to retrieve HTML documents' META tag values to
  - 92) ->(1)(0.625000)  5. Added support for new range query syntax to QueryParser.jj.
*/
    { 4, 6, 207, 202, 206, 208, 209},
/*
4 - 207) ->(1)(0.883883)  the BUILD.txt document to describe how to override the
  - 202) ->(1)(0.625000)  1. Changed QueryParser.jj to have "?" be a special character which
  - 206) ->(1)(0.625000)  1. Renamed build.properties to default.properties and updated
  - 208) ->(1)(0.625000)  default.property settings without having to edit the file. This
  - 209) ->(1)(0.625000)  brings the build process closer to Scarab's build process.
  - 203) ->(1)(0.500000)  allowed it to be used as a wildcard term. Updated TestWildcard
*/
    { 5, 1, 320, -1, -1, -1, -1},
/*
5 - 320) ->(1)(0.875000)  StandardTokenizer is to use StandardAnalyzer.
*/
    { 6, 2, 89, 86, -1, -1, -1},
/*
6 - 89) ->(1)(0.866025)  4. Added id method to Hits to be able to access the index global id.
  - 86) ->(1)(0.625000)  3. Added the ability to disable lock creation by using disableLuceneLocks
*/
    { 7, 5, 41, 43, 46, 48, 45},
/*
7 - 41) ->(1)(0.750000)  1. Added getFieldNames(boolean) to IndexReader, SegmentReader, and
  - 43) ->(1)(0.625000)  2. Changed file locking to place lock files in
  - 46) ->(1)(0.625000)  lock indexes which are read-only to them.
  - 48) ->(1)(0.625000)  permitting one to easily use different analyzers for different
  - 45) ->(1)(0.500000)  permitted to write files.  This way folks can open and correctly
*/
    { 8, 2, 71, 73, -1, -1, -1},
/*
8 - 71) ->(1)(0.750000)  new method, IndexWriter.addIndexes(IndexReader[]), to take
  - 73) ->(1)(0.707107)  12. Added a limit to the number of clauses which may be added to a
*/
    { 9, 4, 185, 180, 184, 182, -1},
/*
9 - 185) ->(1)(0.750000)  (TermQuery, PhraseQuery and BooleanQuery), permitting access to
  - 180) ->(1)(0.625000)  possible for someone to write a Scorer implementation that is
  - 184) ->(1)(0.625000)  g. Added public accessors to the primitive query classes
  - 182) ->(1)(0.500000)  fairly advanced programming, and I don't expect anyone to do
*/
    { 10, 3, 191, 197, 198, -1, -1},
/*
10 - 191) ->(1)(0.750000)  arguments, for easy FSDirectory to RAMDirectory conversion.
   - 197) ->(1)(0.707107)  25. Refactored QueryParser to make it easier for people to extend it.
   - 198) ->(1)(0.625000)  Added the ability to automatically lower-case Wildcard terms in
*/
    { 11, 2, 227, 220, -1, -1, -1},
/*
11 - 227) ->(1)(0.750000)  12. Add escape character to query parser.
   - 220) ->(1)(0.500000)  8. Changed Wildcard search to find 0 or more char instead of 1 or more
*/
    { 12, 3, 235, 236, 239, -1, -1},
/*
12 - 235) ->(1)(0.750000)  Add XML Document #3 implementation to Document Section.
   - 236) ->(1)(0.625000)  Also added Term Highlighting to Misc Section. (carlson)
   - 239) ->(1)(0.625000)  3. Changed document deletion code to obtain the index write lock,
*/
    { 13, 4, 252, 251, 250, 257, -1},
/*
13 - 252) ->(1)(0.750000)  a RAMDirectory index to an FSDirectory.
   - 251) ->(1)(0.625000)  empty index failed.  This was encountered using addIndexes to copy
   - 250) ->(1)(0.500000)  1. IndexWriter: fixed a bug where adding an optimized index to an
   - 257) ->(1)(0.500000)  4. Fix query parser so that PrefixQuery is used in preference to
*/
    { 14, 3, 263, 268, 265, -1, -1},
/*
14 - 263) ->(1)(0.750000)  7. Added 'contributions' section to website & docs. (carlson)
   - 268) ->(1)(0.750000)  ability to reuse TermDocs objects.  (cutting)
   - 265) ->(1)(0.500000)  Folks must now download this separately from metamata in order to
*/
    { 15, 2, 293, 292, -1, -1, -1},
/*
15 - 293) ->(1)(0.750000)  - license switched from LGPL to Apache
   - 292) ->(1)(0.625000)  - packages renamed from com.lucene to org.apache.lucene
*/
    { 16, 1, 3, -1, -1, -1, -1},
/*
16 - 3) ->(1)(0.625000)  1. Added catch of BooleanQuery$TooManyClauses in QueryParser to
*/
    { 17, 1, 6, -1, -1, -1, -1},
/*
17 - 6) ->(1)(0.625000)  3. Added a new method IndexReader.setNorm(), that permits one to
*/
    { 18, 2, 21, 27, -1, -1, -1},
/*
18 - 21) ->(1)(0.625000)  1. Added minMergeDocs in IndexWriter.  This can be raised to speedr
   - 27) ->(1)(0.500000)  4. Fix bug #19253, in demo HTML parser, add whitespace as needed to
*/
    { 19, 1, 109, -1, -1, -1, -1},
/*
19 - 109) ->(1)(0.625000)  11. Changed the German stemming algorithm to ignore case while
*/
    { 20, 3, 131, 133, 137, -1, -1},
/*
20 - 131) ->(1)(0.625000)  used to boost scores of matches on that token.  (cutting)
   - 133) ->(1)(0.500000)  results to only match those which also match a provided query.
   - 137) ->(1)(0.500000)  date field to implement date filtering.  One could re-use a
*/
    { 21, 3, 140, 142, 149, -1, -1},
/*
21 - 140) ->(1)(0.625000)  need to be reconstructed once per day. (cutting)
   - 142) ->(1)(0.625000)  analyzer used when adding documents to this index. (cutting)
   - 149) ->(1)(0.500000)  19. Fixed return of Hits.id() from float to int. (Terry Steichen via Peter).
*/
    { 22, 1, 150, -1, -1, -1, -1},
/*
22 - 150) ->(1)(0.625000)  20. Added getFieldNames() to IndexReader and Segment(s)Reader classes.
*/
    { 23, 2, 164, 167, -1, -1, -1},
/*
23 - 164) ->(1)(0.625000)  like "(+foo +bar)^2 +baz" is now supported and equivalent to
   - 167) ->(1)(0.500000)  query to re-write itself as an alternate, more primitive query.
*/
    { 24, 1, 176, -1, -1, -1, -1},
/*
24 - 176) ->(1)(0.625000)  entire index.  This is intended to be used in developing
*/
    { 25, 1, 218, -1, -1, -1, -1},
/*
25 - 218) ->(1)(0.625000)  7. Added fix to avoid NullPointerException in results.jsp
*/
    { 26, 2, 313, 319, -1, -1, -1},
/*
26 - 313) ->(1)(0.625000)  new tokenizer uses a regular-expression grammar to identify more
   - 319) ->(1)(0.625000)  The easiest way for applications to start using
*/
    { 27, 1, 13, -1, -1, -1, -1},
/*
27 - 13) ->(1)(0.500000)  these cases, the total number of tokens is a better value to use
*/
    { 28, 1, 66, -1, -1, -1, -1},
/*
28 - 66) ->(1)(0.500000)  10. Added Locale setting to QueryParser, for use by date range parsing.
*/
    { 29, 1, 116, -1, -1, -1, -1},
/*
29 - 116) ->(1)(0.500000)  should be re-created from scratch in order for search scores to
*/
    { 30, 1, 127, -1, -1, -1, -1},
/*
30 - 127) ->(1)(0.500000)  and exact phrase query.  This makes it possible, e.g., to build
*/
    { 31, 1, 157, -1, -1, -1, -1},
/*
31 - 157) ->(1)(0.500000)  it possible, e.g., to reuse the same query instance with
*/

    { 0, 0, 0, 0, 0, 0, 0}
};




static void
test_search( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_index_reader_t *reader;
    lcn_searcher_t *searcher;
    lcn_field_t *field;
    lcn_query_t *q;
    lcn_hits_t *hits;
    int i;

    apr_pool_create( &pool, main_pool );

    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        (void) apr_file_remove( file, pool );

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        (void) apr_file_remove( file, pool );
    }

    /* create fsf field: 10 consecutive documents get  */
    /* a common id wich makes them grouped             */

    LCN_TEST( lcn_index_reader_create_by_path( &reader, test_index_name, pool ));

    LCN_TEST( lcn_field_create_fixed_size_by_ints( &field,
                                                   "group",
                                                   0, /* value         */
                                                   0, /* default value */
                                                   8, /* size          */
                                                   pool ));

    LCN_TEST( lcn_index_reader_add_fs_field_def( reader, field ));

    for( i = 0; i < lcn_index_reader_num_docs( reader ); i++ )
    {
        if ( i / 10 != 15 )
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", i / 10 ));
        }
    }

    LCN_TEST( lcn_index_reader_commit( reader ));

    /* initialize searcher with group field */

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );
    lcn_searcher_set_hit_collector_initial_size( searcher, 5 );
    LCN_TEST( lcn_searcher_group_by( searcher, "group" ));
    LCN_TEST( lcn_parse_query( &q, "text:to", pool ));

    nolog( stderr, "\n\nStart SEARCH\n----------------------------------\n" );
    LCN_TEST( lcn_searcher_search( searcher, &hits, q, NULL, pool ));
    nolog( stderr, "\nEND SEARCH\n----------------------------------\n" );

    CuAssertIntEquals( tc, 32, lcn_hits_length( hits ));
    CuAssertIntEquals( tc, 70, lcn_hits_total( hits ));

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* text;
        lcn_score_t sc;
        int j;

        lcn_hits_doc( hits, &doc, i, pool );

        lcn_document_get( doc, &text, "text", pool );
        sc = lcn_document_score( doc );
        //fprintf( stderr, "\n\n---------\n%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
        //lcn_hits_group_hits_size( hits, i ), sc.float_val, text );
        nolog( stderr, "%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
                lcn_hits_group_hits_size( hits, i ), sc.float_val, text );


        CuAssertIntEquals( tc, test_search_data[i][0], i );

        //flog( stderr, "check %d group: %d (%d)\n", i, lcn_hits_group_hits_size( hits, i ),
        //test_search_data[i][2] );
        CuAssertIntEquals( tc, test_search_data[i][1], lcn_hits_group_hits_size( hits, i ));

        for( j = 0; j < lcn_hits_group_hits_size( hits, i ); j++ )
        {
            lcn_document_t *d;
            apr_status_t s = lcn_hits_group_doc( hits, &d, i, j, pool );

            if ( APR_SUCCESS == s )
            {
                lcn_document_get( d, &text, "text", pool );
                nolog( stderr, "check %d/%d -> %d\n", i, j, test_search_data[i][j+2] );
                CuAssertIntEquals( tc, test_search_data[i][j+2], lcn_document_id( d ));
                nolog( stderr, "    %d) %s\n", lcn_document_id( d ), text );
            }
            else
            {
                nolog( stderr, " NOC\n" );
                CuAssertIntEquals( tc, LCN_ERR_GROUP_INDEX_OUT_OF_RANGE, s );

                if ( j < 5 )
                {
                    CuAssertIntEquals( tc, -1, test_search_data[i][j+2] );
                }
            }
        }
    }

    /* test searches */

    /* remove generated fsf file */
    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));
    }

    apr_pool_destroy( pool );
}





int test_search_data_split[][7] =
{
    { 0, 5, 50, 53, 21, 57, 27 },
/*
0 - 50) ->(1)(1.000000)  4. Minor enhancements to FuzzyTermEnum.
  - 53) ->(1)(0.875000)  and MultiIndexSearcher to use it.
  - 21) ->(1)(0.625000)  1. Added minMergeDocs in IndexWriter.  This can be raised to speedr
  - 57) ->(1)(0.625000)  7. Fixed SegmentsReader to eliminate the confusing and slightly different
  - 27) ->(1)(0.500000)  4. Fix bug #19253, in demo HTML parser, add whitespace as needed to
*/
    { 1, 3, 248, 245, 246, -1, -1 },
/*
1 - 248) ->(1)(1.000000)  9. Upgraded to JUnit 3.7. (otis)
  - 245) ->(1)(0.625000)  7. Changed Term and Query to implement Serializable.  (scottganyo)
  - 246) ->(1)(0.625000)  8. Fixed to never delete indexes added with IndexWriter.addIndexes().
*/
    { 2, 1, 278, -1, -1, -1, -1},
/*
2 - 278) ->(1)(1.000000)  - added sources to distribution
*/
    { 3, 3, 97, 94, 92, -1, -1},
/*
3 - 97) ->(1)(0.883883)  7. Modified QueryParser to make it possible to programmatically specify the
  - 94) ->(1)(0.707107)  6. Added the ability to retrieve HTML documents' META tag values to
  - 92) ->(1)(0.625000)  5. Added support for new range query syntax to QueryParser.jj.
*/
    { 4, 6, 207, 202, 206, 208, 209},
/*
4 - 207) ->(1)(0.883883)  the BUILD.txt document to describe how to override the
  - 202) ->(1)(0.625000)  1. Changed QueryParser.jj to have "?" be a special character which
  - 206) ->(1)(0.625000)  1. Renamed build.properties to default.properties and updated
  - 208) ->(1)(0.625000)  default.property settings without having to edit the file. This
  - 209) ->(1)(0.625000)  brings the build process closer to Scarab's build process.
  - 203) ->(1)(0.500000)  allowed it to be used as a wildcard term. Updated TestWildcard
*/
    { 5, 1, 320, -1, -1, -1, -1},
/*
5 - 320) ->(1)(0.875000)  StandardTokenizer is to use StandardAnalyzer.
*/
    { 6, 2, 89, 86, -1, -1, -1},
/*
6 - 89) ->(1)(0.866025)  4. Added id method to Hits to be able to access the index global id.
  - 86) ->(1)(0.625000)  3. Added the ability to disable lock creation by using disableLuceneLocks
*/
    { 7, 5, 41, 43, 46, 48, 45},
/*
7 - 41) ->(1)(0.750000)  1. Added getFieldNames(boolean) to IndexReader, SegmentReader, and
  - 43) ->(1)(0.625000)  2. Changed file locking to place lock files in
  - 46) ->(1)(0.625000)  lock indexes which are read-only to them.
  - 48) ->(1)(0.625000)  permitting one to easily use different analyzers for different
  - 45) ->(1)(0.500000)  permitted to write files.  This way folks can open and correctly
*/
    { 8, 2, 71, 73, -1, -1, -1},
/*
8 - 71) ->(1)(0.750000)  new method, IndexWriter.addIndexes(IndexReader[]), to take
  - 73) ->(1)(0.707107)  12. Added a limit to the number of clauses which may be added to a
*/
    { 9, 4, 185, 180, 184, 182, -1},
/*
9 - 185) ->(1)(0.750000)  (TermQuery, PhraseQuery and BooleanQuery), permitting access to
  - 180) ->(1)(0.625000)  possible for someone to write a Scorer implementation that is
  - 184) ->(1)(0.625000)  g. Added public accessors to the primitive query classes
  - 182) ->(1)(0.500000)  fairly advanced programming, and I don't expect anyone to do
*/
    { 10, 3, 191, 197, 198, -1, -1},
/*
10 - 191) ->(1)(0.750000)  arguments, for easy FSDirectory to RAMDirectory conversion.
   - 197) ->(1)(0.707107)  25. Refactored QueryParser to make it easier for people to extend it.
   - 198) ->(1)(0.625000)  Added the ability to automatically lower-case Wildcard terms in
*/
    { 11, 2, 227, 220, -1, -1, -1},
/*
11 - 227) ->(1)(0.750000)  12. Add escape character to query parser.
   - 220) ->(1)(0.500000)  8. Changed Wildcard search to find 0 or more char instead of 1 or more
*/
    { 12, 3, 235, 236, 239, -1, -1},
/*
12 - 235) ->(1)(0.750000)  Add XML Document #3 implementation to Document Section.
   - 236) ->(1)(0.625000)  Also added Term Highlighting to Misc Section. (carlson)
   - 239) ->(1)(0.625000)  3. Changed document deletion code to obtain the index write lock,
*/
    { 13, 4, 252, 251, 250, 257, -1},
/*
13 - 252) ->(1)(0.750000)  a RAMDirectory index to an FSDirectory.
   - 251) ->(1)(0.625000)  empty index failed.  This was encountered using addIndexes to copy
   - 250) ->(1)(0.500000)  1. IndexWriter: fixed a bug where adding an optimized index to an
   - 257) ->(1)(0.500000)  4. Fix query parser so that PrefixQuery is used in preference to
*/
    { 14, 3, 263, 268, 265, -1, -1},
/*
14 - 263) ->(1)(0.750000)  7. Added 'contributions' section to website & docs. (carlson)
   - 268) ->(1)(0.750000)  ability to reuse TermDocs objects.  (cutting)
   - 265) ->(1)(0.500000)  Folks must now download this separately from metamata in order to
*/
    { 15, 2, 293, 292, -1, -1, -1},
/*
15 - 293) ->(1)(0.750000)  - license switched from LGPL to Apache
   - 292) ->(1)(0.625000)  - packages renamed from com.lucene to org.apache.lucene
*/
    { 16, 1, 3, -1, -1, -1, -1},
/*
16 - 3) ->(1)(0.625000)  1. Added catch of BooleanQuery$TooManyClauses in QueryParser to
*/
    { 17, 1, 6, -1, -1, -1, -1},
/*
17 - 6) ->(1)(0.625000)  3. Added a new method IndexReader.setNorm(), that permits one to
*/
    { 18, 1, 109, -1, -1, -1, -1},
/*
18 - 109) ->(1)(0.625000)  11. Changed the German stemming algorithm to ignore case while
*/
    { 19, 3, 131, 133, 137, -1, -1},
/*
19 - 131) ->(1)(0.625000)  used to boost scores of matches on that token.  (cutting)
   - 133) ->(1)(0.500000)  results to only match those which also match a provided query.
   - 137) ->(1)(0.500000)  date field to implement date filtering.  One could re-use a
*/
    { 20, 3, 140, 142, 149, -1, -1},
/*
20 - 140) ->(1)(0.625000)  need to be reconstructed once per day. (cutting)
   - 142) ->(1)(0.625000)  analyzer used when adding documents to this index. (cutting)
   - 149) ->(1)(0.500000)  19. Fixed return of Hits.id() from float to int. (Terry Steichen via Peter).
*/
    { 21, 1, 150, -1, -1, -1, -1},
/*
21 - 150) ->(1)(0.625000)  20. Added getFieldNames() to IndexReader and Segment(s)Reader classes.
*/
    { 22, 2, 164, 167, -1, -1, -1},
/*
22 - 164) ->(1)(0.625000)  like "(+foo +bar)^2 +baz" is now supported and equivalent to
   - 167) ->(1)(0.500000)  query to re-write itself as an alternate, more primitive query.
*/
    { 23, 1, 176, -1, -1, -1, -1},
/*
23 - 176) ->(1)(0.625000)  entire index.  This is intended to be used in developing
*/
    { 24, 1, 218, -1, -1, -1, -1},
/*
24 - 218) ->(1)(0.625000)  7. Added fix to avoid NullPointerException in results.jsp
*/
    { 25, 2, 313, 319, -1, -1, -1},
/*
25 - 313) ->(1)(0.625000)  new tokenizer uses a regular-expression grammar to identify more
   - 319) ->(1)(0.625000)  The easiest way for applications to start using
*/
    { 26, 1, 13, -1, -1, -1, -1},
/*
26 - 13) ->(1)(0.500000)  these cases, the total number of tokens is a better value to use
*/
    { 27, 1, 66, -1, -1, -1, -1},
/*
27 - 66) ->(1)(0.500000)  10. Added Locale setting to QueryParser, for use by date range parsing.
*/
    { 28, 1, 116, -1, -1, -1, -1},
/*
28 - 116) ->(1)(0.500000)  should be re-created from scratch in order for search scores to
*/
    { 29, 1, 127, -1, -1, -1, -1},
/*
29 - 127) ->(1)(0.500000)  and exact phrase query.  This makes it possible, e.g., to build
*/
    { 30, 1, 157, -1, -1, -1, -1},
/*
30 - 157) ->(1)(0.500000)  it possible, e.g., to reuse the same query instance with
*/
    { 0, 0, 0, 0, 0, 0, 0}
};


static lcn_bool_t
is_split_group_val_1( unsigned int val )
{
    return (val == 5);
}


static void
test_search_split( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_index_reader_t *reader;
    lcn_searcher_t *searcher;
    lcn_field_t *field;
    lcn_query_t *q;
    lcn_hits_t *hits;
    int i;
    int (*test_data)[7] = test_search_data_split;

    apr_pool_create( &pool, main_pool );

    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        (void) apr_file_remove( file, pool );

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        (void) apr_file_remove( file, pool );
    }

    /* create fsf field: 10 consecutive documents get  */
    /* a common id wich makes them grouped             */

    LCN_TEST( lcn_index_reader_create_by_path( &reader, test_index_name, pool ));

    LCN_TEST( lcn_field_create_fixed_size_by_ints( &field,
                                                   "group",
                                                   0, /* value         */
                                                   0, /* default value */
                                                   8, /* size          */
                                                   pool ));

    LCN_TEST( lcn_index_reader_add_fs_field_def( reader, field ));

    for( i = 0; i < lcn_index_reader_num_docs( reader ); i++ )
    {
        if ( 2 == (i / 10))
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", 5 ));
        }
        else if ( i / 10 != 15 )
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", i / 10 ));
        }
    }

    LCN_TEST( lcn_index_reader_commit( reader ));

    /* initialize searcher with group field */

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );
    lcn_searcher_set_hit_collector_initial_size( searcher, 5 );
    LCN_TEST( lcn_searcher_group_by( searcher, "group" ));
    lcn_searcher_set_split_group_func( searcher, is_split_group_val_1 );

    LCN_TEST( lcn_parse_query( &q, "text:to", pool ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, q, NULL, pool ));

    CuAssertIntEquals( tc, 31, lcn_hits_length( hits ));
    CuAssertIntEquals( tc, 70, lcn_hits_total( hits ));

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* text;
        lcn_score_t sc;
        int j;
        lcn_hits_doc( hits, &doc, i, pool );

        lcn_document_get( doc, &text, "text", pool );
        sc = lcn_document_score( doc );
        nolog( stderr, "\n\n---------\n%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
        lcn_hits_group_hits_size( hits, i ), sc.float_val, text );
        nolog( stderr, "%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
                lcn_hits_group_hits_size( hits, i ), sc.float_val, text );

        CuAssertIntEquals( tc, test_data[i][0], i );

        nolog( stderr, "check %d group: %d (%d)\n", i, lcn_hits_group_hits_size( hits, i ),
              test_data[i][2] );
        CuAssertIntEquals( tc, test_data[i][1], lcn_hits_group_hits_size( hits, i ));

        for( j = 0; j < lcn_hits_group_hits_size( hits, i ); j++ )
        {
            lcn_document_t *d;
            apr_status_t s = lcn_hits_group_doc( hits, &d, i, j, pool );

            if ( APR_SUCCESS == s )
            {
                lcn_document_get( d, &text, "text", pool );
                nolog( stderr, "check %d/%d -> %d\n", i, j, test_data[i][j+2] );
                CuAssertIntEquals( tc, test_data[i][j+2], lcn_document_id( d ));
                nolog( stderr, "    %d) %s\n", lcn_document_id( d ), text );
            }
            else
            {
                CuAssertIntEquals( tc, LCN_ERR_GROUP_INDEX_OUT_OF_RANGE, s );

                if ( j < 5 )
                {
                    CuAssertIntEquals( tc, -1, test_data[i][j+2] );
                }
            }
        }
    }

    /* test searches */

    /* remove generated fsf file */
    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));
    }

    apr_pool_destroy( pool );
}


int test_search_data_split_lower[][7] =
{
    { 0, 5, 50, 53, 21, 57, 27 },
/*
0 - 50) ->(1)(1.000000)  4. Minor enhancements to FuzzyTermEnum.
  - 53) ->(1)(0.875000)  and MultiIndexSearcher to use it.
  - 21) ->(1)(0.625000)  1. Added minMergeDocs in IndexWriter.  This can be raised to speedr
  - 57) ->(1)(0.625000)  7. Fixed SegmentsReader to eliminate the confusing and slightly different
  - 27) ->(1)(0.500000)  4. Fix bug #19253, in demo HTML parser, add whitespace as needed to
*/
    { 1, 3, 248, 245, 246, -1, -1 },
/*
1 - 248) ->(1)(1.000000)  9. Upgraded to JUnit 3.7. (otis)
  - 245) ->(1)(0.625000)  7. Changed Term and Query to implement Serializable.  (scottganyo)
  - 246) ->(1)(0.625000)  8. Fixed to never delete indexes added with IndexWriter.addIndexes().
*/
    { 2, 1, 278, -1, -1, -1, -1},
/*
2 - 278) ->(1)(1.000000)  - added sources to distribution
*/
    { 3, 3, 97, 94, 92, -1, -1},
/*
3 - 97) ->(1)(0.883883)  7. Modified QueryParser to make it possible to programmatically specify the
  - 94) ->(1)(0.707107)  6. Added the ability to retrieve HTML documents' META tag values to
  - 92) ->(1)(0.625000)  5. Added support for new range query syntax to QueryParser.jj.
*/
    { 4, 6, 207, 202, 206, 208, 209},
/*
4 - 207) ->(1)(0.883883)  the BUILD.txt document to describe how to override the
  - 202) ->(1)(0.625000)  1. Changed QueryParser.jj to have "?" be a special character which
  - 206) ->(1)(0.625000)  1. Renamed build.properties to default.properties and updated
  - 208) ->(1)(0.625000)  default.property settings without having to edit the file. This
  - 209) ->(1)(0.625000)  brings the build process closer to Scarab's build process.
  - 203) ->(1)(0.500000)  allowed it to be used as a wildcard term. Updated TestWildcard
*/
    { 5, 1, 320, -1, -1, -1, -1},
/*
5 - 320) ->(1)(0.875000)  StandardTokenizer is to use StandardAnalyzer.
*/
    { 6, 2, 89, 86, -1, -1, -1},
/*
6 - 89) ->(1)(0.866025)  4. Added id method to Hits to be able to access the index global id.
  - 86) ->(1)(0.625000)  3. Added the ability to disable lock creation by using disableLuceneLocks
*/
    { 7, 5, 41, 43, 46, 48, 45},
/*
7 - 41) ->(1)(0.750000)  1. Added getFieldNames(boolean) to IndexReader, SegmentReader, and
  - 43) ->(1)(0.625000)  2. Changed file locking to place lock files in
  - 46) ->(1)(0.625000)  lock indexes which are read-only to them.
  - 48) ->(1)(0.625000)  permitting one to easily use different analyzers for different
  - 45) ->(1)(0.500000)  permitted to write files.  This way folks can open and correctly
*/
    { 8, 2, 71, 73, -1, -1, -1},
/*
8 - 71) ->(1)(0.750000)  new method, IndexWriter.addIndexes(IndexReader[]), to take
  - 73) ->(1)(0.707107)  12. Added a limit to the number of clauses which may be added to a
*/
    { 9, 4, 185, 180, 184, 182, -1},
/*
9 - 185) ->(1)(0.750000)  (TermQuery, PhraseQuery and BooleanQuery), permitting access to
  - 180) ->(1)(0.625000)  possible for someone to write a Scorer implementation that is
  - 184) ->(1)(0.625000)  g. Added public accessors to the primitive query classes
  - 182) ->(1)(0.500000)  fairly advanced programming, and I don't expect anyone to do
*/
    { 10, 3, 191, 197, 198, -1, -1},
/*
10 - 191) ->(1)(0.750000)  arguments, for easy FSDirectory to RAMDirectory conversion.
   - 197) ->(1)(0.707107)  25. Refactored QueryParser to make it easier for people to extend it.
   - 198) ->(1)(0.625000)  Added the ability to automatically lower-case Wildcard terms in
*/
    { 11, 2, 227, 220, -1, -1, -1},
/*
11 - 227) ->(1)(0.750000)  12. Add escape character to query parser.
   - 220) ->(1)(0.500000)  8. Changed Wildcard search to find 0 or more char instead of 1 or more
*/
    { 12, 3, 235, 236, 239, -1, -1},
/*
12 - 235) ->(1)(0.750000)  Add XML Document #3 implementation to Document Section.
   - 236) ->(1)(0.625000)  Also added Term Highlighting to Misc Section. (carlson)
   - 239) ->(1)(0.625000)  3. Changed document deletion code to obtain the index write lock,
*/
    { 13, 4, 252, 251, 250, 257, -1},
/*
13 - 252) ->(1)(0.750000)  a RAMDirectory index to an FSDirectory.
   - 251) ->(1)(0.625000)  empty index failed.  This was encountered using addIndexes to copy
   - 250) ->(1)(0.500000)  1. IndexWriter: fixed a bug where adding an optimized index to an
   - 257) ->(1)(0.500000)  4. Fix query parser so that PrefixQuery is used in preference to
*/
    { 14, 3, 263, 268, 265, -1, -1},
/*
14 - 263) ->(1)(0.750000)  7. Added 'contributions' section to website & docs. (carlson)
   - 268) ->(1)(0.750000)  ability to reuse TermDocs objects.  (cutting)
   - 265) ->(1)(0.500000)  Folks must now download this separately from metamata in order to
*/
    { 15, 2, 293, 292, -1, -1, -1},
/*
15 - 293) ->(1)(0.750000)  - license switched from LGPL to Apache
   - 292) ->(1)(0.625000)  - packages renamed from com.lucene to org.apache.lucene
*/
    { 16, 1, 3, -1, -1, -1, -1},
/*
16 - 3) ->(1)(0.625000)  1. Added catch of BooleanQuery$TooManyClauses in QueryParser to
*/
    { 17, 1, 6, -1, -1, -1, -1},
/*
17 - 6) ->(1)(0.625000)  3. Added a new method IndexReader.setNorm(), that permits one to
*/
    { 18, 1, 109, -1, -1, -1, -1},
/*
18 - 109) ->(1)(0.625000)  11. Changed the German stemming algorithm to ignore case while
*/
    { 19, 3, 131, 133, 137, -1, -1},
/*
19 - 131) ->(1)(0.625000)  used to boost scores of matches on that token.  (cutting)
   - 133) ->(1)(0.500000)  results to only match those which also match a provided query.
   - 137) ->(1)(0.500000)  date field to implement date filtering.  One could re-use a
*/
    { 20, 3, 140, 142, 149, -1, -1},
/*
20 - 140) ->(1)(0.625000)  need to be reconstructed once per day. (cutting)
   - 142) ->(1)(0.625000)  analyzer used when adding documents to this index. (cutting)
   - 149) ->(1)(0.500000)  19. Fixed return of Hits.id() from float to int. (Terry Steichen via Peter).
*/
    { 21, 1, 150, -1, -1, -1, -1},
/*
21 - 150) ->(1)(0.625000)  20. Added getFieldNames() to IndexReader and Segment(s)Reader classes.
*/
    { 22, 4, 164, 313, 319, 167, -1},
/*
22 - 164) ->(1)(0.625000)  like "(+foo +bar)^2 +baz" is now supported and equivalent to
   - 313) ->(1)(0.625000)  new tokenizer uses a regular-expression grammar to identify more
   - 319) ->(1)(0.625000)  The easiest way for applications to start using
   - 167) ->(1)(0.500000)  query to re-write itself as an alternate, more primitive query.
*/
    { 23, 1, 176, -1, -1, -1, -1},
/*
23 - 176) ->(1)(0.625000)  entire index.  This is intended to be used in developing
*/
    { 24, 1, 218, -1, -1, -1, -1},
/*
24 - 218) ->(1)(0.625000)  7. Added fix to avoid NullPointerException in results.jsp
*/
    { 25, 1, 13, -1, -1, -1, -1},
/*
25 - 13) ->(1)(0.500000)  these cases, the total number of tokens is a better value to use
*/
    { 26, 1, 66, -1, -1, -1, -1},
/*
26 - 66) ->(1)(0.500000)  10. Added Locale setting to QueryParser, for use by date range parsing.
*/
    { 27, 1, 116, -1, -1, -1, -1},
/*
27 - 116) ->(1)(0.500000)  should be re-created from scratch in order for search scores to
*/
    { 28, 1, 127, -1, -1, -1, -1},
/*
28 - 127) ->(1)(0.500000)  and exact phrase query.  This makes it possible, e.g., to build
*/
    { 29, 1, 157, -1, -1, -1, -1},
/*
29 - 157) ->(1)(0.500000)  it possible, e.g., to reuse the same query instance with
*/
    { 0, 0, 0, 0, 0, 0, 0}
};


static lcn_bool_t
is_split_group_val_2( unsigned int val )
{
    return (val == 5 || val == 16);
}


static void
test_search_split_lower_score( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_index_reader_t *reader;
    lcn_searcher_t *searcher;
    lcn_field_t *field;
    lcn_query_t *q;
    lcn_hits_t *hits;
    int i;
    int (*test_data)[7] = test_search_data_split_lower;

    apr_pool_create( &pool, main_pool );

    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        (void) apr_file_remove( file, pool );

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        (void) apr_file_remove( file, pool );
    }

    /* create fsf field: 10 consecutive documents get  */
    /* a common id wich makes them grouped             */

    LCN_TEST( lcn_index_reader_create_by_path( &reader, test_index_name, pool ));

    LCN_TEST( lcn_field_create_fixed_size_by_ints( &field,
                                                   "group",
                                                   0, /* value         */
                                                   0, /* default value */
                                                   8, /* size          */
                                                   pool ));

    LCN_TEST( lcn_index_reader_add_fs_field_def( reader, field ));

    for( i = 0; i < lcn_index_reader_num_docs( reader ); i++ )
    {
        if ( 2 == (i / 10))
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", 5 ));
        }
        else if ( 31 == ( i / 10 ))
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", 16 ));
        }
        else if ( i / 10 != 15 )
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader, i, "group", i / 10 ));
        }
    }

    LCN_TEST( lcn_index_reader_commit( reader ));

    /* initialize searcher with group field */

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );
    lcn_searcher_set_hit_collector_initial_size( searcher, 5 );
    LCN_TEST( lcn_searcher_group_by( searcher, "group" ));
    lcn_searcher_set_split_group_func( searcher, is_split_group_val_2 );

    LCN_TEST( lcn_parse_query( &q, "text:to", pool ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, q, NULL, pool ));

    CuAssertIntEquals( tc, 30, lcn_hits_length( hits ));
    CuAssertIntEquals( tc, 70, lcn_hits_total( hits ));

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* text;
        lcn_score_t sc;
        int j;
        lcn_hits_doc( hits, &doc, i, pool );

        lcn_document_get( doc, &text, "text", pool );
        sc = lcn_document_score( doc );
        nolog( stderr, "\n\n---------\n%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
        lcn_hits_group_hits_size( hits, i ), sc.float_val, text );
        nolog( stderr, "%u - %d) ->(%d)(%f)  %s\n", i, lcn_document_id( doc ),
                lcn_hits_group_hits_size( hits, i ), sc.float_val, text );

        CuAssertIntEquals( tc, test_data[i][0], i );

        nolog( stderr, "check %d group: %d (%d)\n", i, lcn_hits_group_hits_size( hits, i ),
              test_data[i][2] );
        CuAssertIntEquals( tc, test_data[i][1], lcn_hits_group_hits_size( hits, i ));

        for( j = 0; j < lcn_hits_group_hits_size( hits, i ); j++ )
        {
            lcn_document_t *d;
            apr_status_t s = lcn_hits_group_doc( hits, &d, i, j, pool );

            if ( APR_SUCCESS == s )
            {
                lcn_document_get( d, &text, "text", pool );
                nolog( stderr, "check %d/%d -> %d\n", i, j, test_data[i][j+2] );
                CuAssertIntEquals( tc, test_data[i][j+2], lcn_document_id( d ));
                nolog( stderr, "    %d) %s\n", lcn_document_id( d ), text );
            }
            else
            {
                CuAssertIntEquals( tc, LCN_ERR_GROUP_INDEX_OUT_OF_RANGE, s );

                if ( j < 5 )
                {
                    CuAssertIntEquals( tc, -1, test_data[i][j+2] );
                }
            }
        }
    }

    /* test searches */

    /* remove generated fsf file */
    {
        char *file = apr_pstrcat( pool, test_index_name, "/group.fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));

        file = apr_pstrcat( pool, test_index_name, "/fsf", NULL );
        LCN_TEST( apr_file_remove( file, pool ));
    }

    apr_pool_destroy( pool );
}


CuSuite *make_group_search_suite(void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, setup );

    SUITE_ADD_TEST( s, test_search );
    SUITE_ADD_TEST( s, test_search_split  );
    SUITE_ADD_TEST( s, test_search_split_lower_score  );
    return s;
}
