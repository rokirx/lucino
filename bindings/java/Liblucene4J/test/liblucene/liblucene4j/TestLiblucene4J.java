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
public class TestLiblucene4J extends TestCase {
    private Liblucene4J liblucene4J = null;

    protected void setUp() throws Exception {
        super.setUp();
        liblucene4J = new Liblucene4J();
    }

    protected void tearDown() throws Exception {
        liblucene4J = null;
        super.tearDown();
    }

    public void testInitialize() {
        liblucene4J.initialize();
        /**@todo Testcode eintragen*/
    }

    public void testShutdown() {
        liblucene4J.shutdown();
        /**@todo Testcode eintragen*/
    }

}
