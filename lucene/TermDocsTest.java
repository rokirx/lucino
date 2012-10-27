import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class TermDocsTest
{


    public static void main( String[] args ) throws Exception
    {
        IndexReader reader = IndexReader.open("/home/rk/luc/Liblucene/build/test/test_index_1/");
        Term t = new Term("text", "a");
        TermDocs termDocs = reader.termDocs( t );

        int[] docs = new int[32];
        int[] freqs = new int[32];

        int read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );

        read = termDocs.read( docs, freqs );
        System.out.println( "READ " + read );
    }
}
