import org.apache.lucene.index.*;
import org.apache.lucene.search.*;
import org.apache.lucene.analysis.*;
import org.apache.lucene.document.*;

public class TestSearcher
{


    public static void main( String[] args ) throws Exception
    {
        IndexSearcher searcher = new IndexSearcher("/srv/www/cgi-bin/nv/products/product_1745");

        BooleanQuery volltext = new BooleanQuery();

        volltext.add( new TermQuery( new Term("volltext", "zinso")), BooleanClause.Occur.MUST );
        //volltext.add( new TermQuery( new Term("volltext", "2006")), BooleanClause.Occur.MUST );
        //volltext.add( new TermQuery( new Term("volltext", "90")), BooleanClause.Occur.MUST );

        BooleanQuery ueberschrift = new BooleanQuery();

        volltext.add( new TermQuery( new Term("ueberschrift", "zinso")), BooleanClause.Occur.MUST );
        volltext.add( new TermQuery( new Term("ueberschrift", "2006")), BooleanClause.Occur.MUST );
        volltext.add( new TermQuery( new Term("ueberschrift", "90")), BooleanClause.Occur.MUST );

        BooleanQuery both = new BooleanQuery();

        //ueberschrift.setBoost( 100.0f );

        both.add( volltext,    BooleanClause.Occur.MUST );
        //both.add( ueberschrift, BooleanClause.Occur.SHOULD );

        Hits h = searcher.search( volltext );
        //Hits h = searcher.search( new TermQuery( new Term("volltext", "zinso" )));

        System.out.println( "hits " + h.length() );

        for ( int i = 0; i < Math.min(115, h.length()); i++ )
        {
            Document doc = h.doc(i);
            System.out.println( doc.getField( "ueberschrift" ) );
        }
    }
}
