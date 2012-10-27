package liblucene.liblucene4j.document;

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
public class Document {
    /**
     *
     * @param pool int
     */
    public void create(int pool) {
        LibluceneJNI.getInstance().documentCreate(this, pool);
    }

    public void addField(Field field, int pool) {
        LibluceneJNI.getInstance().documentAddField(this, field, pool);
    }

    public void freeDocument() {
        LibluceneJNI.getInstance().documentFree(this);
    }

    protected void finalize() {
        LibluceneJNI.getInstance().poolDestroy(this.nativePool);
    }


    public int nativeDocument;
    public int nativePool;
}
