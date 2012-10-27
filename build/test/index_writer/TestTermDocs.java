import org.apache.lucene.index.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class TestTermDocs
{
    public static void main( String[] args ) throws Exception
    {
        IndexReader ir = IndexReader.open("/some/path");
        TermDocs td = ir.termDocs( new Term("some_field", "42" ));
        
        System.out.println("next " + td.next());
        System.out.println(td.doc() + ", " + td.freq() );
        td.skipTo( 210 );
        System.out.println(td.doc() + ", " + td.freq() );
        
    }
}
