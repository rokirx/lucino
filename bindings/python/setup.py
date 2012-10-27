from distutils.core import setup, Extension

lucene = Extension('lucene',
                    sources = ['src/lucene.c'],
                    libraries = ['apr-1', 'lucene'],
                    library_dirs=['/usr/local/apr/lib'],
                    include_dirs=['/usr/local/apr/include/apr-1'],)

setup (name = 'lucene',
       version = '0.1',
       description = 'Lucene for Python',
       ext_modules=[lucene])
