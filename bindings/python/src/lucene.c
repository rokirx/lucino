#include <Python.h>
#include <apr_general.h>
#include <lucene.h>
#include <lcn_search.h>
#include <lcn_bitvector.h>
#include "structmember.h"

static PyObject *LuceneError;
static PyTypeObject lucene_BitvectorType;

static PyObject *
Document_new( lcn_hits_t *hits, int index, apr_pool_t *pool );

typedef struct {
    PyObject_HEAD

    /**
     * Searcher Object of Lucene
     */
    lcn_searcher_t *searcher;

    /**
     * Bitvector
     */
    PyObject *bv;

    /**
     *
     */
    apr_pool_t *pool;

} lucene_Searcher;

static PyObject *
Hits_new( lucene_Searcher* parent_searcher, lcn_hits_t *hits, apr_pool_t *pool );

static PyObject *
Searcher_search( lucene_Searcher *self, PyObject *args, PyObject *kwds );

static void
Searcher_dealloc( lucene_Searcher* self)
{
    apr_status_t s;
    char error_buf[1000];

    if ( NULL != self->searcher &&
         ( APR_SUCCESS != ( s = lcn_index_searcher_close( self->searcher ) )))
    {
        lcn_strerror( s, error_buf, 1000 );
        fprintf( stderr, "Error closing index_searcher: '%s'\n", error_buf );
    }

    if ( NULL != self->bv )
    {
        Py_DECREF(self->bv);
    }

    if ( NULL != self->pool )
    {
        apr_pool_destroy( self->pool );
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Searcher_new( PyTypeObject *type, PyObject *args, PyObject *kwds )
{
    PyObject *arg;
    lucene_Searcher *self = NULL;
    char *path = NULL;
    lcn_list_t *path_list = NULL;
    apr_pool_t *pool = NULL;
    apr_status_t s = APR_SUCCESS;
    char error_buf[1000];
    apr_pool_create( &(pool), NULL );


    if (! PyArg_ParseTuple(args, "O", &arg))
    {
        return NULL;
    }

    if ( PyString_Check( arg ))
    {
        if ( NULL == (path = PyString_AsString( arg)))
        {
            return NULL;
        }
    }
    else if ( PyList_Check( arg ))
    {
        int i, path_list_size = PyList_GET_SIZE( arg );
        lcn_list_create( &path_list, path_list_size, pool );

        for( i = 0; i < path_list_size; i++ )
        {
            PyObject *item = PyList_GetItem( arg, i );

            if ( PyString_Check( item ))
            {
                char *p;

                if ( NULL == (p = PyString_AsString( item )))
                {
                    return NULL;
                }

                lcn_list_add( path_list, p );
            }
            else
            {
                return NULL;
            }
        }
    }
    else
    {
        return NULL;
    }

    if ( NULL == (self = (lucene_Searcher *)type->tp_alloc(type, 0)))
    {
        return NULL;
    }

    self->pool = pool;

    if ( NULL != path )
    {
        s = lcn_index_searcher_create_by_path( &(self->searcher), path, self->pool );
    }
    else
    {
        int i;
        lcn_list_t *sub_readers;
        lcn_index_reader_t *multi_reader;

        lcn_list_create( &sub_readers, lcn_list_size( path_list ), self->pool );

        for( i = 0; i < lcn_list_size( path_list) ; i++ )
        {
            lcn_index_reader_t *r;
            lcn_index_reader_create_by_path( &r, (char*) lcn_list_get( path_list, i ), self->pool );
            lcn_list_add( sub_readers, r );
        }

        lcn_multi_reader_create_by_sub_readers( &multi_reader, sub_readers, self->pool );
        lcn_index_searcher_create_by_reader( &(self->searcher), multi_reader, self->pool );
    }

    if ( APR_SUCCESS != s )
    {
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        self->searcher = NULL;
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject *)self;
}

static PyMethodDef Searcher_methods[] = {
    {"search", (PyCFunction)Searcher_search, METH_KEYWORDS, "Execute search with a query" },
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyTypeObject lucene_SearcherType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size           */
    "lucene.Searcher",                        /* tp_name           */
    sizeof(lucene_Searcher),                  /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)Searcher_dealloc,             /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    0,                                        /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags          */
    "Searcher objects",                       /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    Searcher_methods,                         /* tp_methods        */
    0,//Noddy_members,                        /* tp_members        */
    0,                                        /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,//(initproc)Noddy_init,                 /* tp_init           */
    0,                                        /* tp_alloc          */
    Searcher_new,                             /* tp_new            */
};

/* Query */

typedef struct {
    PyObject_HEAD

    /**
     * Searcher Object of Lucene
     */
    lcn_query_t *query;

    /**
     *
     */
    apr_pool_t *pool;

} lucene_Query;

static void
Query_dealloc( lucene_Query* self)
{
    if ( NULL != self->pool )
    {
        apr_pool_destroy( self->pool );
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Query_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    lucene_Query *self = NULL;
    const char *qstring = NULL;
    apr_status_t s;

    if (! PyArg_ParseTuple(args, "s", &qstring))
    {
        return NULL;
    }

    if ( NULL == (self = (lucene_Query *)type->tp_alloc(type, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != apr_pool_create( &(self->pool), NULL))
    {
        Py_DECREF(self);
        return NULL;
    }
    s = lcn_parse_query( &(self->query), qstring, self->pool );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        self->query = NULL;
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject *)self;
}

static PyTypeObject lucene_QueryType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size           */
    "lucene.Query",                           /* tp_name           */
    sizeof(lucene_Query),                     /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)Query_dealloc,                /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    0,                                        /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags          */
    "Query objects",                          /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    0,//Noddy_methods,                        /* tp_methods        */
    0,//Noddy_members,                        /* tp_members        */
    0,                                        /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,//(initproc)Noddy_init,                 /* tp_init           */
    0,                                        /* tp_alloc          */
    Query_new,                                /* tp_new            */
};

#if 0
static PyMethodDef lucene_query_methods[] = {
    {NULL}  /* Sentinel */
};
#endif












/* Hits */

typedef struct {
    PyObject_HEAD

    /**
     * Hits Object of Lucene
     */
    lcn_hits_t *hits;

    /**
     * APR Pool
     */
    apr_pool_t *pool;

    /**
     * Length of hits
     */
    PyObject* length;

    /**
     * Searcher may not be deallocated while Hits is alive
     */
    lucene_Searcher* parent_searcher;

} lucene_Hits;

static void
Hits_dealloc( lucene_Hits* self)
{
    if ( NULL != self->pool )
    {
        apr_pool_destroy( self->pool );
    }

    Py_DECREF(self->length );
    Py_DECREF(self->parent_searcher );

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Hits_doc( lucene_Hits *self, PyObject *args, PyObject *kwds )
{
    PyObject *result = NULL;
    apr_status_t s;
    int docindex;
    apr_pool_t *cp = NULL;

    if (! PyArg_ParseTuple(args, "i", &docindex))
    {
        return NULL;
    }

    if ( APR_SUCCESS != (s = apr_pool_create( &cp, NULL)))
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    if ( NULL == (result = Document_new( self->hits, docindex, cp )))
    {
        apr_pool_destroy( cp );
    }

    return result;
}

static PyObject *
lucene_Hits_getlength( lucene_Hits *self, void *closure )
{
    Py_INCREF(self->length);
    return self->length;
}

static int
lucene_Hits_setlength( lucene_Hits *self, PyObject *value, void *closure)
{
    PyErr_SetString(PyExc_TypeError, "Cannot modify the length attribute");
    return -1;
}

static PyGetSetDef Hits_getseters[] = {
    {"length", (getter) lucene_Hits_getlength, (setter) lucene_Hits_setlength, "length", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef Hits_methods[] = {
    {"doc", (PyCFunction)Hits_doc, METH_KEYWORDS, "Retrieve document n from hits" },
    {NULL}  /* Sentinel */
};


static PyTypeObject lucene_HitsType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size           */
    "lucene.Hits",                            /* tp_name           */
    sizeof(lucene_Hits),                      /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)Hits_dealloc,                 /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    0,                                        /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags          */
    "Hits objects",                           /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    Hits_methods,                             /* tp_methods        */
    0,//Hits_members,                         /* tp_members        */
    Hits_getseters,                           /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,//(initproc)Noddy_init,                 /* tp_init           */
    0,                                        /* tp_alloc          */
    0,                                        /* tp_new            */
};


static PyObject *
Hits_new( lucene_Searcher* parent_searcher, lcn_hits_t *hits, apr_pool_t *pool )
{
    lucene_Hits *self = NULL;

    if ( NULL == (self = (lucene_Hits *)(lucene_HitsType.tp_alloc)(&lucene_HitsType, 0)))
    {
        return NULL;
    }

    self->pool = pool;
    self->hits = hits;
    self->length = PyInt_FromLong( (long) lcn_hits_length( hits ));
    self->parent_searcher = parent_searcher;
    Py_INCREF(self->parent_searcher);

    return (PyObject*) self;
}

#if 0
static PyMethodDef lucene_hits_methods[] = {
    {NULL}  /* Sentinel */
};
#endif








/* Document */

typedef struct {
    PyObject_HEAD

    /**
     * Document Object of Lucene
     */
    lcn_document_t *doc;

    /**
     * APR Pool
     */
    apr_pool_t *pool;

} lucene_Document;

static void
Document_dealloc( lucene_Document* self)
{
    if ( NULL != self->pool )
    {
        apr_pool_destroy( self->pool );
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
lucene_Document_getid( lucene_Document *self, void *closure )
{
    return PyInt_FromLong((long) lcn_document_id( self->doc ) );
}

static int
lucene_Document_setid( lucene_Document *self, PyObject *value, void *closure)
{
    PyErr_SetString(PyExc_TypeError, "Cannot modify the id attribute");
    return -1;
}

static PyObject *
Document_field_value( lucene_Document *self, PyObject *args, PyObject *kwds )
{
    const char* field_name;
    char* value;
    apr_status_t s;

    if (! PyArg_ParseTuple(args, "s", &field_name))
    {
        return NULL;
    }

    s = lcn_document_get( self->doc, &value, field_name, self->pool );

    /* try to read fsf field */

    if ( LCN_ERR_DOCUMENT_FIELD_IS_BINARY == s )
    {
        unsigned int val;
        apr_status_t stat = lcn_document_get_int( self->doc, field_name, &val );

        if ( APR_SUCCESS == stat )
        {
            s = APR_SUCCESS;
            return PyLong_FromUnsignedLong( (unsigned long) ((unsigned int)val) );
        }
    }

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    return PyString_FromString( value );
}

static PyObject *
Document_list_fields( lucene_Document *self, PyObject *args, PyObject *kwds )
{
    PyObject *result = NULL;
    lcn_list_t *field_list;
    unsigned int i;

    if (! PyArg_ParseTuple(args, ""))
    {
        return NULL;
    }

    field_list = lcn_document_get_fields( self->doc );

    result = PyTuple_New( lcn_list_size( field_list ));

    for( i = 0; i < lcn_list_size( field_list ); i++ )
    {
        lcn_field_t *field = lcn_list_get( field_list, i );

        if( PyTuple_SetItem( result, i, PyString_FromString( lcn_field_name( field ))))
        {
            Py_DECREF( result );
            return NULL;
        }
    }

    return result;
}

static PyMethodDef Document_methods[] = {
    {"field_value", (PyCFunction) Document_field_value, METH_KEYWORDS, "Retrieve field value from document" },
    {"list_fields", (PyCFunction) Document_list_fields, METH_KEYWORDS, "List all known document fields" },

    {NULL, NULL }  /* Sentinel */
};

static PyGetSetDef Document_getseters[] = {
    {"id", (getter) lucene_Document_getid, (setter) lucene_Document_setid, "id", NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject lucene_DocumentType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size           */
    "lucene.Document",                        /* tp_name           */
    sizeof(lucene_Document),                  /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)Document_dealloc,             /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    0,                                        /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags          */
    "Document objects",                       /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    Document_methods,                         /* tp_methods        */
    0,//Hits_members,                         /* tp_members        */
    Document_getseters,                       /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,//(initproc)Noddy_init,                 /* tp_init           */
    0,                                        /* tp_alloc          */
    0,                                        /* tp_new            */
};


static PyObject *
Document_new( lcn_hits_t *hits, int index, apr_pool_t *pool )
{
    lucene_Document *self = NULL;
    apr_status_t s;

    if ( NULL == (self = (lucene_Document *)(lucene_DocumentType.tp_alloc)(&lucene_DocumentType, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != ( s = lcn_hits_doc( hits, &(self->doc), index, pool )))
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        Py_DECREF(self);
        return NULL;
    }

    self->pool = pool;
    return (PyObject*) self;
}















/* Bitvector */

typedef struct {
    PyObject_HEAD

    /**
     * Bitvector Object of Lucene
     */
    lcn_bitvector_t *bv;

    /**
     * APR Pool
     */
    apr_pool_t *pool;

} lucene_Bitvector;

static void
Bitvector_dealloc( lucene_Bitvector* self)
{
    if ( NULL != self->pool )
    {
        apr_pool_destroy( self->pool );
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Bitvector_bit( lucene_Bitvector *self, PyObject *args, PyObject *kwds )
{
    int docindex;

    if (! PyArg_ParseTuple(args, "i", &docindex))
    {
        return NULL;
    }

    return PyInt_FromLong(lcn_bitvector_get_bit( self->bv, docindex ) ? 1 : 0 );
}

static PyObject *
Bitvector_unot( lucene_Bitvector *self, PyObject *args, PyObject *kwds )
{
    apr_status_t s;

    if (! PyArg_ParseTuple(args, ""))
    {
        return NULL;
    }

    if ( APR_SUCCESS != (s = lcn_bitvector_not( self->bv )))
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
Bitvector_and( PyObject* oa, PyObject *ob )
{
    apr_status_t s;
    lucene_Bitvector *result;

    if( ! PyObject_IsTrue( oa ) ||
        ! PyObject_TypeCheck( oa, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    if( ! PyObject_IsTrue( ob ) ||
        ! PyObject_TypeCheck( ob, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    /* create new bitvector object */

    if ( NULL == (result = (lucene_Bitvector*) oa->ob_type->tp_alloc(oa->ob_type, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != apr_pool_create( &(result->pool), NULL))
    {
        Py_DECREF(result);
        return NULL;
    }

    s = lcn_bitvector_and( &(((lucene_Bitvector*)result)->bv),
                           ((lucene_Bitvector*)oa)->bv,
                           ((lucene_Bitvector*)ob)->bv,
                           result->pool );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        result->bv = NULL;
        Py_DECREF( result );
        return NULL;
    }

    return (PyObject*) result;
}

static PyObject *
Bitvector_uand( PyObject* oa, PyObject *ob )
{
    apr_status_t s;

    if( ! PyObject_IsTrue( oa ) ||
        ! PyObject_TypeCheck( oa, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    if( ! PyObject_IsTrue( ob ) ||
        ! PyObject_TypeCheck( ob, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    s = lcn_bitvector_uand( ((lucene_Bitvector*)oa)->bv,
                            ((lucene_Bitvector*)ob)->bv );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    return oa;
}

static PyObject *
Bitvector_uor( PyObject* oa, PyObject *ob )
{
    apr_status_t s;

    if( ! PyObject_IsTrue( oa ) ||
        ! PyObject_TypeCheck( oa, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    if( ! PyObject_IsTrue( ob ) ||
        ! PyObject_TypeCheck( ob, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    s = lcn_bitvector_uor( ((lucene_Bitvector*)oa)->bv,
                           ((lucene_Bitvector*)ob)->bv );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    return oa;
}

static PyObject *
Bitvector_or( PyObject* oa, PyObject *ob )
{
    apr_status_t s;
    lucene_Bitvector *result;

    if( ! PyObject_IsTrue( oa ) ||
        ! PyObject_TypeCheck( oa, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    if( ! PyObject_IsTrue( ob ) ||
        ! PyObject_TypeCheck( ob, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    /* create new bitvector object */

    if ( NULL == (result = (lucene_Bitvector*) oa->ob_type->tp_alloc(oa->ob_type, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != apr_pool_create( &(result->pool), NULL))
    {
        Py_DECREF(result);
        return NULL;
    }

    s = lcn_bitvector_or( &(((lucene_Bitvector*)result)->bv),
                          ((lucene_Bitvector*)oa)->bv,
                          ((lucene_Bitvector*)ob)->bv,
                          result->pool );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        result->bv = NULL;
        Py_DECREF( result );
        return NULL;
    }

    return (PyObject*) result;
}

static PyObject *
Bitvector_not( PyObject* oa )
{
    apr_status_t s;
    lucene_Bitvector *result;

    if( ! PyObject_IsTrue( oa ) ||
        ! PyObject_TypeCheck( oa, &lucene_BitvectorType ))
    {
        PyErr_SetString( LuceneError, "Illegal Operation on lucene.Bitvector" );
        return NULL;
    }

    /* create new bitvector object */

    if ( NULL == (result = (lucene_Bitvector*) oa->ob_type->tp_alloc(oa->ob_type, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != apr_pool_create( &(result->pool), NULL))
    {
        Py_DECREF(result);
        return NULL;
    }

    s = lcn_bitvector_clone( &(((lucene_Bitvector*)result)->bv),
                             ((lucene_Bitvector*)oa)->bv,
                             result->pool );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        result->bv = NULL;
        Py_DECREF( result );
        return NULL;
    }

    s = lcn_bitvector_not( ((lucene_Bitvector*)result)->bv );

    if ( APR_SUCCESS != s )
    {
        char error_buf[1000];
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        result->bv = NULL;
        Py_DECREF( result );
        return NULL;
    }

    return (PyObject*) result;
}


static PyMethodDef Bitvector_methods[] = {
    {"unot",    (PyCFunction) Bitvector_unot, METH_KEYWORDS, "Negates all bits of the bitvector"   },
    {"bit",     (PyCFunction) Bitvector_bit,  METH_KEYWORDS, "Retrieve the nth bit of a bitvector" },
    {NULL, NULL }  /* Sentinel */
};

static PyObject *
lucene_Bitvector_getcount( lucene_Bitvector *self, void *closure )
{
    return PyInt_FromLong((long) lcn_bitvector_count( self->bv ) );
}

static int
lucene_Bitvector_setcount( lucene_Bitvector *self, PyObject *value, void *closure)
{
    PyErr_SetString(PyExc_TypeError, "Cannot modify the count attribute");
    return -1;
}

static PyObject *
lucene_Bitvector_getsize( lucene_Bitvector *self, void *closure )
{
    return PyInt_FromLong((long) lcn_bitvector_size( self->bv ) );
}

static int
lucene_Bitvector_setsize( lucene_Bitvector *self, PyObject *value, void *closure)
{
    PyErr_SetString(PyExc_TypeError, "Cannot modify the size attribute");
    return -1;
}

static PyGetSetDef Bitvector_getseters[] = {
    {"count", (getter) lucene_Bitvector_getcount, (setter) lucene_Bitvector_setcount, "length", NULL},
    {"size",  (getter) lucene_Bitvector_getsize,  (setter) lucene_Bitvector_setsize,  "length", NULL},
    {NULL}  /* Sentinel */
};

static PyObject *
Bitvector_new( PyTypeObject *type, PyObject *args, PyObject *kwds )
{
    lucene_Bitvector *self = NULL;
    const char *path = NULL;
    apr_status_t s;
    char error_buf[1000];

    if (! PyArg_ParseTuple(args, "s", &path ))
    {
        return NULL;
    }

    if ( NULL == (self = (lucene_Bitvector *)type->tp_alloc(type, 0)))
    {
        return NULL;
    }

    if ( APR_SUCCESS != apr_pool_create( &(self->pool), NULL))
    {
        Py_DECREF(self);
        return NULL;
    }

    s = lcn_bitvector_create_from_file ( &(self->bv), path, self->pool );

    if ( APR_SUCCESS != s )
    {
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        self->bv = NULL;
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject *)self;
}


static PyNumberMethods Bitvector_number_methods = {
        0,                             /* nb_add                  */
        0,                             /* nb_subtract             */
        0,                             /* nb_multiply             */
        0,                             /* nb_divide               */
        0,                             /* nb_remainder            */
        0,                             /* nb_divmod               */
        0,                             /* nb_power                */
        0,//(unaryfunc)int_neg,        /* nb_negative             */
        0,                             /* nb_positive             */
        0,                             /* nb_absolute             */
        0,                             /* nb_nonzero              */
        (unaryfunc)Bitvector_not,      /* nb_invert               */
        0,                             /* nb_lshift               */
        0,                             /* nb_rshift               */
        (binaryfunc) Bitvector_and,    /* nb_and                  */
        0,//(binaryfunc)Bitvector_xor, /* nb_xor                  */
        (binaryfunc) Bitvector_or,     /* nb_or                   */
        0,//int_coerce,                /* nb_coerce               */
        0,                             /* nb_int                  */
        0,                             /* nb_long                 */
        0,                             /* nb_float                */
        0,                             /* nb_oct                  */
        0,                             /* nb_hex                  */
        0,                             /* nb_inplace_add          */
        0,                             /* nb_inplace_subtract     */
        0,                             /* nb_inplace_multiply     */
        0,                             /* nb_inplace_divide       */
        0,                             /* nb_inplace_remainder    */
        0,                             /* nb_inplace_power        */
        0,                             /* nb_inplace_lshift       */
        0,                             /* nb_inplace_rshift       */
        (binaryfunc) Bitvector_uand,   /* nb_inplace_and          */
        0,                             /* nb_inplace_xor          */
        (binaryfunc) Bitvector_uor,    /* nb_inplace_or           */
        0,                             /* nb_floor_divide         */
        0,                             /* nb_true_divide          */
        0,                             /* nb_inplace_floor_divide */
        0,                             /* nb_inplace_true_divide  */
//        0,                           /* nb_index                */
};

static PyTypeObject lucene_BitvectorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size           */
    "lucene.Bitvector",                       /* tp_name           */
    sizeof(lucene_Bitvector),                 /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)Bitvector_dealloc,            /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    &Bitvector_number_methods,                 /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags          */
    "Bitvector objects",                      /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    Bitvector_methods,                        /* tp_methods        */
    0,//Bitvector_members,                    /* tp_members        */
    Bitvector_getseters,                      /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,//(initproc)Noddy_init,                 /* tp_init           */
    0,                                        /* tp_alloc          */
    Bitvector_new,                            /* tp_new            */
};

static PyObject *
Searcher_search( lucene_Searcher *self, PyObject *args, PyObject *kwds )
{
    PyObject *result = NULL;
    lcn_query_t *query;
    apr_status_t s;
    apr_pool_t *hits_pool = NULL;
    char error_buf[1000];
    const char *qstring;
    static char *kwlist[] = {"query", "bitvector", NULL };
    PyObject *bitvector = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|O", kwlist, &qstring, &bitvector ))
    {
        return NULL;
    }

    if( NULL != self->bv )
    {
        Py_DECREF(self->bv);
        self->bv = NULL;
    }

    if ( NULL != bitvector &&
         PyObject_IsTrue( bitvector ) &&
         PyObject_TypeCheck( bitvector, &lucene_BitvectorType ))
    {
        self->bv = bitvector;
        Py_INCREF(self->bv);
    }

    if ( APR_SUCCESS != ( s = apr_pool_create( &hits_pool, NULL )))
    {
        lcn_strerror( s, error_buf, 1000 );
        PyErr_SetString( LuceneError, error_buf );
        return NULL;
    }

    do
    {
        lcn_hits_t* hits;
        lucene_Bitvector *bv = (lucene_Bitvector*)self->bv;

        if ( APR_SUCCESS != (s = lcn_parse_query( &query, qstring, hits_pool )))
        {
            lcn_strerror( s, error_buf, 1000 );
            PyErr_SetString( LuceneError, error_buf );
            break;
        }

        if ( APR_SUCCESS != (s = lcn_searcher_search( self->searcher,
                                                      &hits,
                                                      query,
                                                      NULL != bv ? bv->bv : NULL,
                                                      hits_pool )))
        {
            lcn_strerror( s, error_buf, 1000 );
            PyErr_SetString( LuceneError, error_buf );
            break;
        }

        result = Hits_new( self, hits, hits_pool );
    }
    while(0);

    if ( APR_SUCCESS != s && NULL != hits_pool )
    {
        apr_pool_destroy( hits_pool );
    }

    return result;
}
















#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initlucene(void)
{
    PyObject* m = Py_InitModule3("lucene",
                                 Searcher_methods,
                                 "Example module that creates an extension type.");

    /* Add Searcher Type */

    if (PyType_Ready(&lucene_SearcherType) < 0)
    {
        return;
    }
    Py_INCREF(&lucene_SearcherType);
    PyModule_AddObject(m, "Searcher", (PyObject *)&lucene_SearcherType);


    /* Add Query Type */

    if (PyType_Ready(&lucene_QueryType) < 0)
    {
        return;
    }

    Py_INCREF(&lucene_QueryType);
    PyModule_AddObject(m, "Query",    (PyObject *)&lucene_QueryType);

    /* Add Hits Type */

    if (PyType_Ready(&lucene_HitsType) < 0)
    {
        return;
    }

    Py_INCREF(&lucene_HitsType);
    PyModule_AddObject(m, "Hits",    (PyObject *)&lucene_HitsType);

    /* Add Document Type */

    if (PyType_Ready(&lucene_DocumentType) < 0)
    {
        return;
    }

    Py_INCREF(&lucene_DocumentType);
    PyModule_AddObject(m, "Document", (PyObject *)&lucene_DocumentType);

    /* Add Bitvector Type */

    if (PyType_Ready(&lucene_BitvectorType) < 0)
    {
        return;
    }

    Py_INCREF(&lucene_BitvectorType);
    PyModule_AddObject(m, "Bitvector", (PyObject *)&lucene_BitvectorType);

    {
        /* init apr and lucene stuff */
        apr_pool_t *pool;
        apr_status_t s;
        char error_buf[1000];

        LuceneError = PyErr_NewException("lucene.error", NULL, NULL);
        Py_INCREF(LuceneError);
        PyModule_AddObject(m, "error", LuceneError);

        if ( ( APR_SUCCESS != ( s = apr_initialize())))
        {
            lcn_strerror( s, error_buf, 1000 );
            fprintf( stderr, "Error initializing apr: '%s'\n", error_buf );
        }

        if ( ( APR_SUCCESS != ( s = apr_pool_create( &pool, NULL ))))
        {
            lcn_strerror( s, error_buf, 1000 );
            fprintf( stderr, "Error creating apr-pool: '%s'\n", error_buf );
        }

        if ( ( APR_SUCCESS != ( s = lcn_atom_init(pool) )))
        {
            lcn_strerror( s, error_buf, 1000 );
            fprintf( stderr, "Error initializing lucene: '%s'\n", error_buf );
        }
    }
}
