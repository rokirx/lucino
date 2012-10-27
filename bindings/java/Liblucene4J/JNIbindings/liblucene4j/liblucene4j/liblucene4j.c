#include "liblucene4j.h"

#define DEBUG_PRINT(msg) if (DEBUG) printf("%s\n", msg);

/**
* Macro: store pointer in java object to referencing native object
*/
#define JAVA_STORE_POINTER_FIELD(env, javaclass, fieldname, value) {                            \
    jfieldID field = NULL;                                                                      \
    field = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, javaclass), fieldname, "I");    \
    if (field==NULL || (*env)->ExceptionOccurred(env)) {                                        \
        JAVA_THROW_EXCEPTION(env, "Exception getting field from object");                       \
    }                                                                                           \
    (*env)->SetIntField(env, javaclass, field, (jint)value);                                    \
    if ((*env)->ExceptionOccurred(env)) {                                                       \
       JAVA_THROW_EXCEPTION(env, "Exception setting field in object");                          \
    }                                                                                           \
}

/**
* Macro: get pointer to referencing native object from java object
*/
#define JAVA_GET_POINTER_FIELD(env, javaclass, fieldname, value) {                              \
    jfieldID field = NULL;                                                                      \
    field = (*env)->GetFieldID( env,                                                            \
                                (*env)->GetObjectClass(env, javaclass),                         \
                                fieldname,                                                      \
                                "I");                                                           \
    if (field==NULL || (*env)->ExceptionOccurred(env)) {                                        \
        JAVA_THROW_EXCEPTION(env, "Exception getting field from object");                       \
    }                                                                                           \
    (*value) = (*env)->GetIntField(env, javaclass, field);                                      \
    if ((*value)==NULL || (*env)->ExceptionOccurred(env)) {                                     \
        JAVA_THROW_EXCEPTION(env, "Exception getting value from field");                        \
    }                                                                                           \
}

/**
* Convert java strings into apr-ize'd C-Strings
*/
void
convert_java_string(JNIEnv *env, 
                    jobject jobj, 
                    jstring src, 
                    char** target, 
                    apr_pool_t* pool)
{
    do {
        char* src_string;
        src_string = (char*)calloc(1, (*env)->GetStringLength(env, src));
        src_string = (char*)(*env)->GetStringUTFChars(env, src, NULL);
        (*target) = apr_pstrdup(pool, src_string);

        (*env)->ReleaseStringUTFChars(env, src, src_string);

    } while(FALSE);
}

/**
* Init LibluceneJNI, create parent pool etc.
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_initLibluceneJNI(JNIEnv *env, 
                                                                jobject jobj, 
                                                                jobject java_liblucene4j)
{
    apr_pool_t* pool;
    apr_initialize();

    do {
        apr_pool_create(&pool, NULL);
        lcn_atom_init(pool);
    
        DEBUG_PRINT("enter init");
        
        JAVA_STORE_POINTER_FIELD(env, java_liblucene4j, "pool", pool);

        DEBUG_PRINT("leave init");
    } while(FALSE);
}

/**
*
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_shutdownLibluceneJNI(JNIEnv *env, 
                                                                    jobject jobj,
                                                                    jobject java_liblucene4j)
{
    do {
        DEBUG_PRINT("enter shutdown");
        apr_terminate();
        DEBUG_PRINT("leave shutdown");
    } while(FALSE);
}

/**
* Destroy given pool
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_poolDestroy(JNIEnv *env, 
                                                        jobject jobj, 
                                                        jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        apr_pool_t* pool;
        if (ref_pool==NULL) {
            JAVA_THROW_EXCEPTION(env, "Pool ungueltig in poolDestroy");
        }
        pool = (apr_pool_t*)ref_pool;

        printf("gebe pool %x frei\n", pool);
        
        apr_pool_destroy(pool);
        printf("pool freigegeben\n");

    }while(FALSE);
    return s;
}

/**
* index_writer_create_by_path
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterCreateByPath(JNIEnv *env, 
                                                                       jobject jobj,
                                                                       jobject java_indexwriter, 
                                                                       jstring path, 
                                                                       jboolean create, 
                                                                       jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        apr_pool_t* pool;
        apr_pool_t* child_pool;
        lcn_index_writer_t *lcn_indexwriter;

        char* path_string = NULL;
        
        DEBUG_PRINT("enter index_writer_create_by_path");
        pool = (apr_pool_t*)ref_pool;

        apr_pool_create(&child_pool, pool);

        /* Convert path to C-String */
        convert_java_string(env, jobj, path, &path_string, child_pool);

        /* Create index writer */
        LCNCE(lcn_index_writer_create_by_path(&lcn_indexwriter, path_string, create, child_pool));

        /* Store pointer */
        JAVA_STORE_POINTER_FIELD(env, java_indexwriter, "nativeIndexWriter", lcn_indexwriter);
        DEBUG_PRINT("leave index_writer_create_by_path\n");

    }while(FALSE);

    /* Throw exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create index writer");
    }
}

/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterAddDocument(JNIEnv *env,
                                                                       jobject jobj,
                                                                       jobject java_indexwriter, 
                                                                       jobject java_document)
{
    apr_status_t s = APR_SUCCESS;
    do {
        jint field_native_document;
        jint field_native_indexwriter;
        lcn_document_t* lcn_document;
        lcn_index_writer_t* lcn_index_writer;
        
        DEBUG_PRINT("enter index_writer_add_doc");

        /* Get pointers from java-Objects */
        JAVA_GET_POINTER_FIELD(env, java_document, "nativeDocument", &field_native_document);
        lcn_document = (lcn_document_t*)field_native_document;

        JAVA_GET_POINTER_FIELD(env, java_indexwriter, "nativeIndexWriter", &field_native_indexwriter);
        lcn_index_writer = (lcn_index_writer_t*)field_native_indexwriter;
        
        /* Add document to index */
        LCNCE(lcn_index_writer_add_document(lcn_index_writer, lcn_document));
        DEBUG_PRINT("leave index_writer_add_doc");
    } while(FALSE);

    /* Catch exception on errors */
    if (s!=APR_SUCCESS) {
        fprintf(stderr, "errorcode: %d\n", s);
        JAVA_THROW_EXCEPTION(env, "Exception in adding document to index writer:");
    }
}

/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterClose(JNIEnv *env, 
                                                                 jobject jobj, 
                                                                 jobject java_indexwriter)
{
    apr_status_t s = APR_SUCCESS;
    do {
        jint field_native_indexwriter;
        lcn_index_writer_t* lcn_index_writer;
        DEBUG_PRINT("enter index_writer_close");

        /* Get native index_writer*/
        JAVA_GET_POINTER_FIELD(env, java_indexwriter, "nativeIndexWriter", &field_native_indexwriter);        
        lcn_index_writer = (lcn_index_writer_t*)field_native_indexwriter;

        /* Close index writer */
        LCNCE(lcn_index_writer_close(lcn_index_writer));
        DEBUG_PRINT("leave index_writer_close");

    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot close index writer");
    }
}


JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterOptimize(JNIEnv *env, 
                                                                jobject jobj, 
                                                                jobject java_indexwriter)
{
    apr_status_t s = APR_SUCCESS;
    do {
        jint field_native_indexwriter;
        lcn_index_writer_t* lcn_index_writer;
        DEBUG_PRINT("enter index_writer_optimize");

        /* Get native index_writer*/
        JAVA_GET_POINTER_FIELD(env, java_indexwriter, "nativeIndexWriter", &field_native_indexwriter);        
        lcn_index_writer = (lcn_index_writer_t*)field_native_indexwriter;

        /* optimize index writer */        
        LCNCE(lcn_index_writer_optimize(lcn_index_writer));
        DEBUG_PRINT("leave index_writer_optimize");

    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot optimize index writer");
    }
}


/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_documentCreate(JNIEnv *env, 
                                                               jobject jobj, 
                                                               jobject java_document, 
                                                               jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        lcn_document_t* lcn_document;
        apr_pool_t* child_pool ;
        apr_pool_t* pool = (apr_pool_t*)ref_pool;        
        DEBUG_PRINT("enter document_create");
        apr_pool_create(&child_pool, pool);

        DEBUG_PRINT(apr_psprintf(pool, "child_pool document: %x\n", child_pool));

        /* create document */
        LCNCE(lcn_document_create(&lcn_document, child_pool));

        /* store pointer */
        JAVA_STORE_POINTER_FIELD(env, java_document, "nativeDocument", lcn_document);

        JAVA_STORE_POINTER_FIELD(env, java_document, "nativePool", child_pool);

        DEBUG_PRINT("leave document_create"); 
    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create document");
    }
}


/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_documentFree(JNIEnv *env, 
                                                         jobject jobj, 
                                                         jobject java_document)
{
    apr_status_t s = APR_SUCCESS;
    do {
        jint field_native_document;
        lcn_document_t* lcn_document;
        lcn_list_t *document_fields;
        int i = 0;
            
         /* Get pointers from java-Objects */
        JAVA_GET_POINTER_FIELD(env, java_document, "nativeDocument", &field_native_document);
        lcn_document = (lcn_document_t*)field_native_document;            
#if 0
        document_fields = lcn_document_get_fields(lcn_document);

        DEBUG_PRINT("vor den fields");

        /* Free fields */
        for (i=0; i<lcn_list_size(document_fields); i++) {
            lcn_field_t* field = lcn_list_get(document_fields, i);

            DEBUG_PRINT(apr_psprintf(lcn_document_pool(lcn_document), 
                                        "gebe frei: %d %s %x\n", 
                                        i, 
                                        lcn_field_name(field),
                                        lcn_field_pool(field)));
            apr_pool_destroy(lcn_field_pool(field));
        }
        DEBUG_PRINT(apr_psprintf(lcn_document_pool(lcn_document), "gebe document frei: %x\n", lcn_document_pool(lcn_document)));
#endif
        /* Free document */
        apr_pool_destroy(lcn_document_pool(lcn_document));

    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create document");
    }
}

/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_documentAddField(JNIEnv *env, 
                                                                jobject jobj, 
                                                                jobject java_document, 
                                                                jobject java_field,
                                                                jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        jint field_native_document;
        jint field_native_field;
        lcn_document_t* lcn_document;
        lcn_field_t* lcn_field;
        
        DEBUG_PRINT("enter document_add_field");

        /* Get pointers from java-Objects */
        JAVA_GET_POINTER_FIELD(env, java_document, "nativeDocument", &field_native_document);
        lcn_document = (lcn_document_t*)field_native_document;

        JAVA_GET_POINTER_FIELD(env, java_field, "nativeField", &field_native_field);
        lcn_field = (lcn_field_t*)field_native_field;
        
        /* Add field to document */
        LCNCE(lcn_document_add_field(lcn_document, lcn_field, NULL)); 

        if ((*env)->NewWeakGlobalRef(env, lcn_field)!=NULL) {
            DEBUG_PRINT("create weak global ref");
        }

        DEBUG_PRINT("leave document_add_field");

    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create document");
    }
}


/**
* 
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldCreate(JNIEnv *env, 
                                                            jobject jobj, 
                                                            jobject java_field, 
                                                            jstring name, 
                                                            jstring value, 
                                                            jint fieldflags, 
                                                            jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        lcn_field_t* lcn_field;
        apr_pool_t* child_pool;        
        char* namestr = NULL;
        char* valuestr = NULL;

        apr_pool_t* pool = (apr_pool_t*)ref_pool;
        
        DEBUG_PRINT("enter field_create");
        apr_pool_create(&child_pool, pool);

        DEBUG_PRINT(apr_psprintf(pool, "child_pool field: %x\n", child_pool));

        /* Get C-Strings from Java-String */
        convert_java_string(env, jobj, name, &namestr, child_pool);
        convert_java_string(env, jobj, value, &valuestr, child_pool);

        /* Create field */        
        LCNCE(lcn_field_create(&lcn_field, namestr, valuestr, fieldflags, LCN_FIELD_VALUE_COPY, child_pool));

        /* Store pointer */
        JAVA_STORE_POINTER_FIELD(env, java_field, "nativeField", lcn_field);

        JAVA_STORE_POINTER_FIELD(env, java_field, "nativePool", child_pool);

        DEBUG_PRINT("leave field_create");
    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create field");
    }
}


/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldBinaryCreate(JNIEnv *env, 
                                                              jobject jobj, 
                                                              jobject java_field, 
                                                              jstring name, 
                                                              jbyte* value, 
                                                              jint fieldsize, 
                                                              jint fieldflags, 
                                                              jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        lcn_field_t* lcn_field;
        apr_pool_t* child_pool;
        char* namestr = NULL;
        char *valuestr = NULL;
        int i = 0;

        apr_pool_t* pool = (apr_pool_t*)ref_pool;
        
        DEBUG_PRINT("enter field_binary_create");
        apr_pool_create(&child_pool, pool);

        /* Get C-Strings from Java-String */
        convert_java_string(env, jobj, name, &namestr, child_pool);
        valuestr = apr_pcalloc(child_pool, sizeof(char)*fieldsize);
        for (i=0; i<fieldsize; i++) {
            valuestr[i] = (char)value[i];
        }
        valuestr[i] = '\0';

        LCNCE(lcn_field_create_binary(&lcn_field, namestr, valuestr, fieldflags, LCN_FIELD_VALUE_COPY, fieldsize, child_pool));

        /* Create field */        
        LCNCE(lcn_field_create(&lcn_field, namestr, valuestr, fieldflags, LCN_FIELD_VALUE_COPY, child_pool));

        /* Store pointer */
        JAVA_STORE_POINTER_FIELD(env, java_field, "nativeField", lcn_field);
        JAVA_STORE_POINTER_FIELD(env, java_field, "nativePool", child_pool);

        DEBUG_PRINT("leave field_binary_create");
    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Cannot create field");
    }
}


/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldSetAnalyzer(JNIEnv *env, 
                                                                 jobject jobj, 
                                                                 jobject java_field, 
                                                                 jobject java_analyzer)
{
   do {
        jint field_native_analyzer;
        jint field_native_field;
        lcn_analyzer_t* lcn_analyzer;
        lcn_field_t* lcn_field;

        DEBUG_PRINT("enter field_set_analyzer");

        /* Get pointers from java-Objects */
        JAVA_GET_POINTER_FIELD(env, java_field, "nativeField", &field_native_field);
        lcn_field = (lcn_field_t*)field_native_field;

        JAVA_GET_POINTER_FIELD(env, java_analyzer, "nativeAnalyzer", &field_native_analyzer);
        lcn_analyzer = (lcn_analyzer_t*)field_native_analyzer;

        /* Set analyzer */
        lcn_field_set_analyzer(lcn_field, lcn_analyzer);

        DEBUG_PRINT("leave field_set_analyzer");

    }while(FALSE);
}

/**
*
*/
JNIEXPORT void JNICALL 
Java_liblucene_liblucene4j_jni_LibluceneJNI_germanAnalyzerCreate(JNIEnv *env, 
                                                                     jobject jobj, 
                                                                     jobject java_german_analyzer, 
                                                                     jint ref_pool)
{
    apr_status_t s = APR_SUCCESS;
    do {
        lcn_analyzer_t* lcn_german_analyzer;
        
        apr_pool_t* child_pool ;
        apr_pool_t* pool = (apr_pool_t*)ref_pool;        
        DEBUG_PRINT("enter german_analyzer_create");
        apr_pool_create(&child_pool, pool);

        /* Create analyzer */
        LCNCE(lcn_german_analyzer_create(&lcn_german_analyzer, child_pool));

        /* Store pointer */
        JAVA_STORE_POINTER_FIELD(env, java_german_analyzer, "nativeAnalyzer", lcn_german_analyzer);
        DEBUG_PRINT("leave german_analyzer_create"); 
    }while(FALSE);

    /* Catch exception */
    if (s!=APR_SUCCESS) {
        JAVA_THROW_EXCEPTION(env, "Exception creating analyzer");
    }
}
