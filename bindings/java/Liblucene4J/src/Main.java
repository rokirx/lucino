

import liblucene.liblucene4j.analysis.Analyzer;
import liblucene.liblucene4j.analysis.GermanAnalyzer;
import liblucene.liblucene4j.document.Document;
import liblucene.liblucene4j.document.Field;
import liblucene.liblucene4j.index.IndexWriter;
import liblucene.liblucene4j.Liblucene4J;

/**
 *
 * <p></p>
 *
 * <p></p>
 *
 * <p>Copyright: Liblucene project (c) 2006</p>
 *
 * <p><a href="http://liblucene.sf.net">http://liblucene.sf.net</a></p>
 *
 * @author V. Klein
 * @version 1.0
 */
public class Main {
    public Main() {
    }

    public static void main(String[] args) {

        boolean init = false;
        Liblucene4J liblucene4j = Liblucene4J.getInstance();

        try {

            liblucene4j.initialize();
            init = true;

            /* IndexWriter */
            /* createByPath(path, create, pool)*/
            IndexWriter indexWriter = new IndexWriter();
            indexWriter.createByPath("C:\\blapath", true, liblucene4j.getPool());

            /* create document */
            /* create(pool) */
            Document document = new Document();
            document.create(liblucene4j.getPool());

            /* create field */
            /* create(name, value, flags, pool)*/
            Field field1 = new Field();
            field1.create("field1", "value1", Field.LCN_FIELD_STORED, liblucene4j.getPool());
            Field field2 = new Field();
            field2.create("field2", "Das ist ein TestTestTest", Field.LCN_FIELD_INDEXED | Field.LCN_FIELD_STORED, liblucene4j.getPool());

            Field field3 = new Field();
            field3.createBinary("field3", "Testvalue".getBytes(), Field.LCN_FIELD_BINARY|Field.LCN_FIELD_STORED, liblucene4j.getPool());

            /* create GermanAnalyzer */
            GermanAnalyzer germanAnalyzer = new GermanAnalyzer();
            germanAnalyzer.createAnalyzer(liblucene4j.getPool());

            /* set field Analyzer */
            field2.fieldSetAnalyzer((Analyzer)germanAnalyzer);

            /* add fields to document */
            /* addField(field, pool) */
            document.addField(field1, liblucene4j.getPool());
            document.addField(field2, liblucene4j.getPool());
            document.addField(field3, liblucene4j.getPool());


            /* add document to writer */
            /* addDocument(document) */
            indexWriter.addDocument(document);

            /* close IndexWriter (and write index) */
            indexWriter.close();

            indexWriter.optimize();

            /* Shutdown module - MUST! */
            liblucene4j.shutdown();
            System.out.println("Done.");

        } catch (java.lang.Throwable ex) {
            System.err.println(ex.getMessage());
            /* APR-Terminate immer aufrufen */
            if (init) {
                System.out.println("running shutdown...");
                liblucene4j.shutdown();
                System.out.println("Done.");
            }
        }

    }
}
