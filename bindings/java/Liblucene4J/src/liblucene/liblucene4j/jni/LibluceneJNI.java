package liblucene.liblucene4j.jni;

import liblucene.liblucene4j.Liblucene4J;
import liblucene.liblucene4j.analysis.Analyzer;
import liblucene.liblucene4j.analysis.GermanAnalyzer;
import liblucene.liblucene4j.document.Document;
import liblucene.liblucene4j.document.Field;
import liblucene.liblucene4j.index.IndexWriter;


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
public class LibluceneJNI {
    protected LibluceneJNI() {
    }

    public static synchronized LibluceneJNI getInstance() {
        if (instance == null) {
            instance = new LibluceneJNI();
        }
        return instance;
    }

    static {
        try {

            System.loadLibrary("lucene4j");
        } catch (Exception ex) {
            System.err.println("Cannot load liblucene4j library");
        }
    }

    /**
     * Native function
     * @param liblucene4j Liblucene4J
     */
    public native void initLibluceneJNI(Liblucene4J liblucene4j);

    public native void shutdownLibluceneJNI(Liblucene4J liblucene4j);

    public native void poolDestroy(int pool);

    public native void indexWriterCreateByPath(IndexWriter indexwriter,
                                               String path, boolean create,
                                               int pool);

    public native void indexWriterAddDocument(IndexWriter indexwriter,
                                              Document document);

    public native void indexWriterClose(IndexWriter indexwriter);

    public native void indexWriterOptimize(IndexWriter indexwriter);

    public native void documentCreate(Document document, int pool);

    public native void documentAddField(Document document, Field field,
                                        int pool);

    public native void documentAddBinaryField(Document document, Field field,
                                              int pool);

    public native void documentFree(Document document);

    public native void fieldCreate(Field field, java.lang.String name,
                                   String value, int flags,
                                   int pool);

    public native void fieldBinaryCreate(Field field, java.lang.String name,
                                         byte[] value, int size, int flags,
                                         int pool);

    public native void fieldSetAnalyzer(Field field, Analyzer analyzer);

    public native void germanAnalyzerCreate(GermanAnalyzer analyzer, int pool);

    private static LibluceneJNI instance = null;
}
