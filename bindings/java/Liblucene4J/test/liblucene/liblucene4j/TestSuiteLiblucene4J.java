package liblucene.liblucene4j;

import junit.framework.*;

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
public class TestSuiteLiblucene4J extends TestCase {

    public TestSuiteLiblucene4J(String s) {
        super(s);
    }

    public static Test suite() {
        TestSuite suite = new TestSuite();
        return suite;
    }
}
