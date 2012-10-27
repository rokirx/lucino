import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class IndexWriter01
{
    public static void main( String[] args ) throws Exception
    {
        IndexWriter writer = new IndexWriter( args[0], new SimpleAnalyzer(), true );
        writer.setMergeFactor(7);
        writer.setMaxBufferedDocs(11);
        writer.setUseCompoundFile( false );

        //LCN_TEST( lcn_field_create( &field, "text", "open source", LCN_FIELD_STORED, pool ) );

        Document doc = new Document();

        doc.add( new Field( "text", "open source", Field.Store.YES, Field.Index.NO ) );
        writer.addDocument( doc );
        writer.close();
    }
}
