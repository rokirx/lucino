package liblucene.liblucene4j.analysis;

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
public class GermanAnalyzer extends Analyzer {
    public GermanAnalyzer() {

    }

    public void createAnalyzer(int pool) {
        LibluceneJNI.getInstance().germanAnalyzerCreate(this, pool);
    }

}
