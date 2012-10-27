package liblucene.liblucene4j.index;

import liblucene.liblucene4j.document.Document;
import liblucene.liblucene4j.jni.LibluceneJNI;

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
public class IndexWriter {
    public IndexWriter() {

    }
    /**
     * Erstellt einen neuen IndexWriter
     * @param path String
     * @param create boolean
     * @param pool int
     */
    public void createByPath(String path, boolean create, int pool) {
        LibluceneJNI.getInstance().indexWriterCreateByPath(this, path, create, pool);
    }

    public void addDocument(Document document) {
        LibluceneJNI.getInstance().indexWriterAddDocument(this, document);
    }

    public void close() {
        LibluceneJNI.getInstance().indexWriterClose(this);
    }

    public void optimize() {
        LibluceneJNI.getInstance().indexWriterOptimize(this);
    }

    public int nativeIndexWriter;
}
