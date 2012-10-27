import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class IndexWriterStress
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
        //writer.setMergeFactor(4);
        //writer.setMaxBufferedDocs(3);
        writer.setUseCompoundFile( false );

        for( int i = 0; i < 64000; i++ )
        {
            addDocument( writer, Integer.toString(i, 36), Integer.toString(i, 10) );
        }

        writer.close();
    }
}
