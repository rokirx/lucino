import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class IndexWriter08
{
    public static void main( String[] args ) throws Exception
    {
        IndexWriter writer = new IndexWriter( args[0], new SimpleAnalyzer(), true );
        writer.setMergeFactor(7);
        writer.setMaxBufferedDocs(11);
        writer.setUseCompoundFile( false );

        //LCN_TEST( lcn_field_create( &field, "text", "open source", LCN_FIELD_INDEXED, pool ) );

        Document doc = new Document();
        doc.add( new Field( "text", (new String("öaß")).getBytes("ISO-8859-1"), Field.Store.YES ));
        doc.add( new Field( "titel", "first", Field.Store.NO, Field.Index.UN_TOKENIZED ));
        writer.addDocument( doc );
        writer.close();
    }
}
