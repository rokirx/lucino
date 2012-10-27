package liblucene.liblucene4j.document;

import liblucene.liblucene4j.analysis.Analyzer;
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
public class Field {
    public Field() {
    }

    /**
     *
     * @param name String
     * @param value String
     * @param flags int
     * @param pool int
     */
    public void create(java.lang.String name, String value, int flags,
                       int pool) {

        LibluceneJNI.getInstance().fieldCreate(this, name, value, flags, pool);
    }

    public void createBinary(java.lang.String name, byte[] value, int flags,
                       int pool) {

        LibluceneJNI.getInstance().fieldBinaryCreate(this, name, value, value.length, flags, pool);
    }

    public void fieldSetAnalyzer(Analyzer analyzer) {
        LibluceneJNI.getInstance().fieldSetAnalyzer(this, analyzer);
    }

    protected void finalize() {
        LibluceneJNI.getInstance().poolDestroy(this.nativePool);
    }

    public int nativeField;
    public int nativePool;

    /**
     * Some constants
     */
    public static int LCN_FIELD_TOKENIZED = 0x1;
    public static int LCN_FIELD_BINARY = 0x2;
    public static int LCN_FIELD_COMPRESSED = 0x4;
    public static int LCN_FIELD_INDEXED = 0x8;
    public static int LCN_FIELD_OMIT_NORMS = 0x10;
    public static int LCN_FIELD_STORED = 0x20;
    public static int LCN_FIELD_STORE_TERM_VECTOR = 0x40;
    public static int LCN_FIELD_STORE_OFFSET_WITH_TV = 0x80;
    public static int LCN_FIELD_STORE_POSITION_WITH_TV = 0x100;
    public static int LCN_FIELD_FIXED_SIZE = 0x200;

}
