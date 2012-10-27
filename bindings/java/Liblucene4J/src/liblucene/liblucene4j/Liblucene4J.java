package liblucene.liblucene4j;

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
public class Liblucene4J {

    protected Liblucene4J() {
    }

    public static synchronized Liblucene4J getInstance() {
        if (instance == null) {
            instance = new Liblucene4J();
        }
        return instance;
    }

    public void initialize() {
        /* Initialize module */
        LibluceneJNI.getInstance().initLibluceneJNI(this);
    }

    public void shutdown() {
        LibluceneJNI.getInstance().shutdownLibluceneJNI(this);
    }

    public int getPool() {
        return pool;
    }

    public int pool;

    private static Liblucene4J instance = null;
}
