import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class IndexWriter10
{

    private static void addDocument( IndexWriter writer, String textVal, String titleVal ) throws Exception
    {
        Document doc = new Document();
        doc.add( new Field( "text", textVal.getBytes("ISO-8859-1"), Field.Store.YES ));
        doc.add( new Field( "titel", titleVal, Field.Store.NO, Field.Index.NO_NORMS ));
        writer.addDocument( doc );
    }
        
    public static void main( String[] args ) throws Exception
    {
        IndexWriter writer = new IndexWriter( args[0], new SimpleAnalyzer(), true );
        writer.setMergeFactor(4);
        writer.setMaxBufferedDocs(3);
        writer.setUseCompoundFile( false );

        addDocument( writer, "öaß", "first" );
        addDocument( writer, "123Ö", "second" );
        addDocument( writer, "$%&", "third" );
        addDocument( writer, "ö1ß", "4th" );
        addDocument( writer, "133Ö", "5th" );

        writer.close();
    }
}
