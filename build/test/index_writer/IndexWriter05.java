import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class IndexWriter05
{
    public static void main( String[] args ) throws Exception
    {
        IndexWriter writer = new IndexWriter( args[0], new SimpleAnalyzer(), true );
        writer.setMergeFactor(7);
        writer.setMaxBufferedDocs(11);
        writer.setUseCompoundFile( false );

        //LCN_TEST( lcn_field_create( &field, "text", "open source", LCN_FIELD_INDEXED, pool ) );

        Document doc = new Document();
        doc.add( new Field( "text", "open", Field.Store.NO, Field.Index.TOKENIZED ) );
        doc.add( new Field( "titel", "simple analyzer", Field.Store.NO, Field.Index.TOKENIZED ) );
        writer.addDocument( doc );
        writer.close();
    }
}
