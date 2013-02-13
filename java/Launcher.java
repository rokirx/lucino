package trunk.lucene;

import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.IndexWriterConfig;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.SimpleFSDirectory;
import org.apache.lucene.util.Version;

import java.io.File;
import java.io.IOException;

public class Launcher
{

    /**
     * Small example create Index with no documentes.
     * Dependencies: lucene-core
     */
    public static void main(String[] args) throws IOException
    {
        File file = new File(System.getProperty("user.home") + "/lucene");
        System.out.println("Index location: " + file.getPath());

        Directory dir = new SimpleFSDirectory(file);
        IndexWriter indexWriter = new IndexWriter(dir, new IndexWriterConfig(Version.LUCENE_50, null));

        indexWriter.commit();
        indexWriter.close();
    }

}
