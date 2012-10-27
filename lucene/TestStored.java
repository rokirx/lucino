import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class TestStored
{
    public static void main( String[] args ) throws Exception
    {
        IndexWriter writer = new IndexWriter( args[0], new SimpleAnalyzer(), true );

        writer.setUseCompoundFile( false );
        Document doc = new Document();
        doc.add( new Field( "field1", "value", Field.Store.NO, Field.Index.UN_TOKENIZED ) );
        doc.add( new Field( "field2", "value", Field.Store.NO, Field.Index.TOKENIZED ) );
        //doc.add( new Field( "field3", "value", Field.Store.YES, Field.Index.TOKENIZED ) );
        //doc.add( new Field( "field4", "value", Field.Store.YES, Field.Index.UN_TOKENIZED ) );

        writer.addDocument( doc );
        writer.close();

        IndexReader reader = IndexReader.open( args[0] );
        doc = reader.document(0);
        Field f1 = doc.getField("field1");
        Field f2 = doc.getField("field2");

        System.out.println( "Field f1 is tokenized: " + f1.isTokenized());
        System.out.println( "Field f2 is tokenized: " + f2.isTokenized());

        reader.close();
    }
}
