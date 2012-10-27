import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class TestIndex
{


    public static void main( String[] args ) throws Exception
    {
        System.out.println(Integer.MAX_VALUE);

        writer.setMergeFactor(7);
        writer.setMaxBufferedDocs(11);
        writer.setUseCompoundFile( false );

        for( int i = 0; i < data.length; i++ )
        {
            Document doc = new Document();

            doc.add( new Field( "id", "KW" + i, Field.Store.YES, Field.Index.UN_TOKENIZED ) );
            doc.add( new Field( "content", data[i], Field.Store.YES, Field.Index.NO ) );
            doc.add( new Field( "text", data[i], Field.Store.YES, Field.Index.TOKENIZED) );

            writer.addDocument( doc );
        }

        writer.close();

        /*
        IndexReader reader = IndexReader.open( "/home/rk/luc/Liblucene/build/test/test_index_1" );
        Term t = new Term("text", "a");
        TermDocs td = reader.termDocs( t );

        int i = 0;

        while( td.next() )
        {

            System.out.println( i + " <" + td.doc() + ", " + td.freq() + ">" );
            i++;
        }

        reader.close();
        */
    }

    private static String[] data = {
        "Lucene Change Log",
        "$Id: TestIndex.java,v 1.6 2006/01/05 00:14:26 rokirchg Exp $",
        "1.3 final",
        "1. Added catch of BooleanQuery$TooManyClauses in QueryParser to",
        "throw ParseException instead. (Erik Hatcher)",
        "2. Fixed a NullPointerException in Query.explain(). (Doug Cutting)",        // 5

        "3. Added a new method IndexReader.setNorm(), that permits one to",          // 6
        "alter the boosting of fields after an index is created.",

        "4. Distinguish between the final position and length when indexing a",      // 8
        "field.  The length is now defined as the total number of tokens,",
        "instead of the final position, as it was previously.  Length is",
        "used for score normalization (Similarity.lengthNorm()) and for",
        "controlling memory usage (IndexWriter.maxFieldLength).  In both of",
        "these cases, the total number of tokens is a better value to use",          // 13
        "than the final token position.  Position is used in phrase",
        "searching (see PhraseQuery and Token.setPositionIncrement()).",

        "5. Fix StandardTokenizer's handling of CJK characters (Chinese,",
        "Japanese and Korean ideograms).  Previously contiguous sequences",
        "were combined in a single token, which is not very useful.  Now",           // 18
        "each ideogram generates a separate token, which is more useful.",           // 19


        "1.3 RC 3",

        "1. Added minMergeDocs in IndexWriter.  This can be raised to speed",
        "indexing without altering the number of files, but only using more",
        "memory.  (Julien Nioche via Otis)",

        "2. Fix bug #24786, in query rewriting. (bschneeman via Cutting)",

        "3. Fix bug #16952, in demo HTML parser, skip comments in",
        "javascript. (Christoph Goller)",

        "4. Fix bug #19253, in demo HTML parser, add whitespace as needed to",
        "output (Daniel Naber via Christoph Goller)",

        "5. Fix bug #24301, in demo HTML parser, long titles no longer",
        "hang things. (Christoph Goller)",

        "6. Fix bug #23534, Replace use of file timestamp of segments file",
        "with an index version number stored in the segments file.  This",
        "resolves problems when running on file systems with low-resolution",
        "timestamps, e.g., HFS under MacOS X.  (Christoph Goller)",

        "7. Fix QueryParser so that TokenMgrError is not thrown, only",
        "ParseException.  (Erik Hatcher)",

        "8. Fix some bugs introduced by change 11 of RC 2.  (Christoph Goller)",

        "9. Fixed a problem compiling TestRussianStem.  (Christoph Goller)",        // 38

        "10. Cleaned up some build stuff.  (Erik Hatcher)",


        "1.3 RC 2",

        "1. Added getFieldNames(boolean) to IndexReader, SegmentReader, and",
        "SegmentsReader. (Julien Nioche via otis)",

        "2. Changed file locking to place lock files in",
        "System.getProperty(\"java.io.tmpdir\"), where all users are",
        "permitted to write files.  This way folks can open and correctly",
        "lock indexes which are read-only to them.",

        "3. IndexWriter: added a new method, addDocument(Document, Analyzer),",     // 47
        "permitting one to easily use different analyzers for different",
        "documents in the same index.",

        "4. Minor enhancements to FuzzyTermEnum.",
        "(Christoph Goller via Otis)",

        "5. PriorityQueue: added insert(Object) method and adjusted IndexSearcher",
        "and MultiIndexSearcher to use it.",
        "(Christoph Goller via Otis)",

        "6. Fixed a bug in IndexWriter that returned incorrect docCount().",       // 55
        "(Christoph Goller via Otis)",

        "7. Fixed SegmentsReader to eliminate the confusing and slightly different",
        "behaviour of TermEnum when dealing with an enumeration of all terms,",
        "versus an enumeration starting from a specific term.",                    // 59
        "This patch also fixes incorrect term document frequences when the same term",
        "is present in multiple segments.",
        "(Christoph Goller via Otis)",

        "8. Added CachingWrapperFilter and PerFieldAnalyzerWrapper. (Erik Hatcher)",

        "9. Added support for the new \"compound file\" index format (Dmitry",
        "Serebrennikov)",

        "10. Added Locale setting to QueryParser, for use by date range parsing.",

        "11. Changed IndexReader so that it can be subclassed by classes",
        "outside of its package.  Previously it had package-private",
        "abstract methods.  Also modified the index merging code so that it",
        "can work on an arbitrary IndexReader implementation, and added a",      // 70
        "new method, IndexWriter.addIndexes(IndexReader[]), to take",
        "advantage of this. (cutting)",

        "12. Added a limit to the number of clauses which may be added to a",    // 73
        "BooleanQuery.  The default limit is 1024 clauses.  This should",
        "stop most OutOfMemoryExceptions by prefix, wildcard and fuzzy",
        "queries which run amok. (cutting)",

        "13. Add new method: IndexReader.undeleteAll().  This undeletes all",
        "deleted documents which still remain in the index. (cutting)",          // 78


        "1.3 RC 1",

        "1. Fixed PriorityQueue's clear() method.",
        "Fix for bug 9454, http://nagoya.apache.org/bugzilla/show_bug.cgi?id=9454",
        "(Matthijs Bomhoff via otis)",

        "2. Changed StandardTokenizer.jj grammar for EMAIL tokens.",
        "Fix for bug 9015, http://nagoya.apache.org/bugzilla/show_bug.cgi?id=9015",
        "(Dale Anson via otis)",

        "3. Added the ability to disable lock creation by using disableLuceneLocks",
        "system property.  This is useful for read-only media, such as CD-ROMs.",
        "(otis)",

        "4. Added id method to Hits to be able to access the index global id.",
        "Required for sorting options.",
        "(carlson)",

        "5. Added support for new range query syntax to QueryParser.jj.",
        "(briangoetz)",

        "6. Added the ability to retrieve HTML documents' META tag values to",
        "HTMLParser.jj.",
        "(Mark Harwood via otis)",

        "7. Modified QueryParser to make it possible to programmatically specify the",
        "default Boolean operator (OR or AND).",
        "(P�ter Hal�csy via otis)",

        "8. Made many search methods and classes non-final, per requests.",
        "This includes IndexWriter and IndexSearcher, among others.",
        "(cutting)",

        "9. Added class RemoteSearchable, providing support for remote",
        "searching via RMI.  The test class RemoteSearchableTest.java",
        "provides an example of how this can be used.  (cutting)",

        "10. Added PhrasePrefixQuery (and supporting MultipleTermPositions).  The",
        "test class TestPhrasePrefixQuery provides the usage example.",
        "(Anders Nielsen via otis)",

        "11. Changed the German stemming algorithm to ignore case while",
        "stripping. The new algorithm is faster and produces more equal",
        "stems from nouns and verbs derived from the same word.",
        "(gschwarz)",

        "12. Added support for boosting the score of documents and fields via",
        "the new methods Document.setBoost(float) and Field.setBoost(float).",

        "Note: This changes the encoding of an indexed value.  Indexes",
        "should be re-created from scratch in order for search scores to",
        "be correct.  With the new code and an old index, searches will",
        "yield very large scores for shorter fields, and very small scores",
        "for longer fields.  Once the index is re-created, scores will be",
        "as before. (cutting)",

        "13. Added new method Token.setPositionIncrement().",

        "This permits, for the purpose of phrase searching, placing",
        "multiple terms in a single position.  This is useful with",
        "stemmers that produce multiple possible stems for a word.",

        "This also permits the introduction of gaps between terms, so that",
        "terms which are adjacent in a token stream will not be matched by",
        "and exact phrase query.  This makes it possible, e.g., to build",
        "an analyzer where phrases are not matched over stop words which",
        "have been removed.",

        "Finally, repeating a token with an increment of zero can also be",
        "used to boost scores of matches on that token.  (cutting)",

        "14. Added new Filter class, QueryFilter.  This constrains search",
        "results to only match those which also match a provided query.",
        "Results are cached, so that searches after the first on the same",
        "index using this filter are very fast.",

        "This could be used, for example, with a RangeQuery on a formatted",
        "date field to implement date filtering.  One could re-use a",
        "single QueryFilter that matches, e.g., only documents modified",
        "within the last week.  The QueryFilter and RangeQuery would only",
        "need to be reconstructed once per day. (cutting)",

        "15. Added a new IndexWriter method, getAnalyzer().  This returns the",
        "analyzer used when adding documents to this index. (cutting)",

        "16. Fixed a bug with IndexReader.lastModified().  Before, document",
        "deletion did not update this.  Now it does.  (cutting)",

        "17. Added Russian Analyzer.",
        "(Boris Okner via otis)",

        "18. Added a public, extensible scoring API.  For details, see the",
        "javadoc for org.apache.lucene.search.Similarity.",

        "19. Fixed return of Hits.id() from float to int. (Terry Steichen via Peter).",

        "20. Added getFieldNames() to IndexReader and Segment(s)Reader classes.",
        "(Peter Mularien via otis)",

        "21. Added getFields(String) and getValues(String) methods.",
        "Contributed by Rasik Pandey on 2002-10-09",
        "(Rasik Pandey via otis)",

        "22. Revised internal search APIs.  Changes include:",

        "a. Queries are no longer modified during a search.  This makes",
        "it possible, e.g., to reuse the same query instance with",
        "multiple indexes from multiple threads.",

        "b. Term-expanding queries (e.g. PrefixQuery, WildcardQuery,",
        "etc.)  now work correctly with MultiSearcher, fixing bugs 12619",
        "and 12667.",

        "c. Boosting BooleanQuery's now works, and is supported by the",
        "query parser (problem reported by Lee Mallabone).  Thus a query",
        "like \"(+foo +bar)^2 +baz\" is now supported and equivalent to",
        "\"(+foo^2 +bar^2) +baz\".",

        "d. New method: Query.rewrite(IndexReader).  This permits a",
        "query to re-write itself as an alternate, more primitive query.",
        "Most of the term-expanding query classes (PrefixQuery,",
        "WildcardQuery, etc.) are now implemented using this method.",

        "e. New method: Searchable.explain(Query q, int doc).  This",
        "returns an Explanation instance that describes how a particular",
        "document is scored against a query.  An explanation can be",
        "displayed as either plain text, with the toString() method, or",
        "as HTML, with the toHtml() method.  Note that computing an",
        "explanation is as expensive as executing the query over the",
        "entire index.  This is intended to be used in developing",
        "Similarity implementations, and, for good performance, should",
        "not be displayed with every hit.",

        "f. Scorer and Weight are public, not package protected.  It now",
        "possible for someone to write a Scorer implementation that is",
        "not in the org.apache.lucene.search package.  This is still",
        "fairly advanced programming, and I don't expect anyone to do",
        "this anytime soon, but at least now it is possible.",

        "g. Added public accessors to the primitive query classes",
        "(TermQuery, PhraseQuery and BooleanQuery), permitting access to",
        "their terms and clauses.",

        "Caution: These are extensive changes and they have not yet been",
        "tested extensively.  Bug reports are appreciated.",
        "(cutting)",

        "23. Added convenience RAMDirectory constructors taking File and String",
        "arguments, for easy FSDirectory to RAMDirectory conversion.",
        "(otis)",

        "24. Added code for manual renaming of files in FSDirectory, since it",
        "has been reported that java.io.File's renameTo(File) method sometimes",
        "fails on Windows JVMs.",
        "(Matt Tucker via otis)",

        "25. Refactored QueryParser to make it easier for people to extend it.",
        "Added the ability to automatically lower-case Wildcard terms in",
        "the QueryParser.",
        "(Tatu Saloranta via otis)",


        "1.2 RC 6",

        "1. Changed QueryParser.jj to have \"?\" be a special character which",
        "allowed it to be used as a wildcard term. Updated TestWildcard",
        "unit test also. (Ralf Hettesheimer via carlson)",

        "1.2 RC 5",

        "1. Renamed build.properties to default.properties and updated",
        "the BUILD.txt document to describe how to override the",
        "default.property settings without having to edit the file. This",
        "brings the build process closer to Scarab's build process.",
        "(jon)",

        "2. Added MultiFieldQueryParser class. (Kelvin Tan, via otis)",

        "3. Updated \"powered by\" links. (otis)",

        "4. Fixed instruction for setting up JavaCC - Bug #7017 (otis)",

        "5. Added throwing exception if FSDirectory could not create diectory",
        "- Bug #6914 (Eugene Gluzberg via otis)",

        "6. Update MultiSearcher, MultiFieldParse, Constants, DateFilter,",
        "LowerCaseTokenizer javadoc (otis)",

        "7. Added fix to avoid NullPointerException in results.jsp",
        "(Mark Hayes via otis)",

        "8. Changed Wildcard search to find 0 or more char instead of 1 or more",
        "(Lee Mallobone, via otis)",

        "9. Fixed error in offset issue in GermanStemFilter - Bug #7412",
        "(Rodrigo Reyes, via otis)",

        "10. Added unit tests for wildcard search and DateFilter (otis)",

        "11. Allow co-existence of indexed and non-indexed fields with the same name",
        "(cutting/casper, via otis)",

        "12. Add escape character to query parser.",
        "(briangoetz)",

        "13. Applied a patch that ensures that searches that use DateFilter",
        "don't throw an exception when no matches are found. (David Smiley, via",
        "otis)",

        "14. Fixed bugs in DateFilter and wildcardquery unit tests. (cutting, otis, carlson)",


        "1.2 RC 4",

        "1. Updated contributions section of website.",
        "Add XML Document #3 implementation to Document Section.",
        "Also added Term Highlighting to Misc Section. (carlson)",

        "2. Fixed NullPointerException for phrase searches containing",
        "unindexed terms, introduced in 1.2 RC 3.  (cutting)",

        "3. Changed document deletion code to obtain the index write lock,",
        "enforcing the fact that document addition and deletion cannot be",
        "performed concurrently.  (cutting)",

        "4. Various documentation cleanups.  (otis, acoliver)",

        "5. Updated \"powered by\" links.  (cutting, jon)",

        "6. Fixed a bug in the GermanStemmer.  (Bernhard Messer, via otis)",

        "7. Changed Term and Query to implement Serializable.  (scottganyo)",

        "8. Fixed to never delete indexes added with IndexWriter.addIndexes().",
        "(cutting)",

        "9. Upgraded to JUnit 3.7. (otis)",

        "1.2 RC 3",

        "1. IndexWriter: fixed a bug where adding an optimized index to an",
        "empty index failed.  This was encountered using addIndexes to copy",
        "a RAMDirectory index to an FSDirectory.",

        "2. RAMDirectory: fixed a bug where RAMlcn_istream_t could not read",
        "across more than across a single buffer boundary.",

        "3. Fix query parser so it accepts queries with unicode characters.",
        "(briangoetz)",

        "4. Fix query parser so that PrefixQuery is used in preference to",
        "WildcardQuery when there's only an asterisk at the end of the",
        "term.  Previously PrefixQuery would never be used.",

        "5. Fix tests so they compile; fix ant file so it compiles tests",
        "properly.  Added test cases for Analyzers and PriorityQueue.",

        "6. Updated demos, added Getting Started documentation. (acoliver)",

        "7. Added 'contributions' section to website & docs. (carlson)",

        "8. Removed JavaCC from source distribution for copyright reasons.",
        "Folks must now download this separately from metamata in order to",
        "compile Lucene.  (cutting)",

        "9. Substantially improved the performance of DateFilter by adding the",
        "ability to reuse TermDocs objects.  (cutting)",

        "10. Added IndexReader methods:",
        "public static boolean indexExists(String directory);",
        "public static boolean indexExists(File directory);",
        "public static boolean indexExists(Directory directory);",
        "public static boolean isLocked(Directory directory);",
        "public static void unlock(Directory directory);",
        "(cutting, otis)",

        "11. Fixed bugs in GermanAnalyzer (gschwarz)",


        "1.2 RC 2, 19 October 2001:",
        "- added sources to distribution",
        "- removed broken build scripts and libraries from distribution",
        "- SegmentsReader: fixed potential race condition",
        "- FSDirectory: fixed so that getDirectory(xxx,true) correctly",
        "erases the directory contents, even when the directory",
        "has already been accessed in this JVM.",
        "- RangeQuery: Fix issue where an inclusive range query would",
        "include the nearest term in the index above a non-existant",
        "specified upper term.",
        "- SegmentTermEnum: Fix NullPointerException in clone() method",
        "when the Term is null.",
        "- JDK 1.1 compatibility fix: disabled lock files for JDK 1.1,",
        "since they rely on a feature added in JDK 1.2.",

        "1.2 RC 1 (first Apache release), 2 October 2001:",
        "- packages renamed from com.lucene to org.apache.lucene",
        "- license switched from LGPL to Apache",
        "- ant-only build -- no more makefiles",
        "- addition of lock files--now fully thread & process safe",
        "- addition of German stemmer",
        "- MultiSearcher now supports low-level search API",
        "- added RangeQuery, for term-range searching",
        "- Analyzers can choose tokenizer based on field name",
        "- misc bug fixes.",

        "1.01 b (last Sourceforge release), 2 July 2001",
        ". a few bug fixes",
        ". new Query Parser",
        ". new prefix query (search for \"foo*\" matches \"food\")",

        "1.0, 2000-10-04",

        "This release fixes a few serious bugs and also includes some",
        "performance optimizations, a stemmer, and a few other minor",
        "enhancements.",

        "0.04 2000-04-19",

        "Lucene now includes a grammar-based tokenizer, StandardTokenizer.",

        "The only tokenizer included in the previous release (LetterTokenizer)",
        "identified terms consisting entirely of alphabetic characters.  The",
        "new tokenizer uses a regular-expression grammar to identify more",
        "complex classes of terms, including numbers, acronyms, email",
        "addresses, etc.",

        "StandardTokenizer serves two purposes:",

        "1. It is a much better, general purpose tokenizer for use by",
        "applications as is.",

        "The easiest way for applications to start using",
        "StandardTokenizer is to use StandardAnalyzer.",

        "2. It provides a good example of grammar-based tokenization.",

        "If an application has special tokenization requirements, it can",
        "implement a custom tokenizer by copying the directory containing",
        "the new tokenizer into the application and modifying it",
        "accordingly.",

        "0.01, 2000-03-30",

        "First open source release.",

        "The code has been re-organized into a new package and directory",
        "structure for this release.  It builds OK, but has not been tested",
        "beyond that since the re-organization."
    };
}
