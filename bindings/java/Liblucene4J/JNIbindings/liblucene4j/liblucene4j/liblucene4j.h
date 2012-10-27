#include <stdio.h>
#include <jni.h>
#include <errno.h>

#include <apr.h>

#include <lucene.h>
#include <lcn_index.h>
#include <lcn_analysis.h>

#ifndef liblucene4j_h
#define liblucene4j_h

#ifndef _DEBUG
#define DEBUG 0
#else
#define DEBUG 1
#endif

#define JAVA_THROW_EXCEPTION(env, msg) {\
    jclass newExcCls; \
    newExcCls = (*env)->FindClass(env, "java/lang/Throwable"); \
    if (newExcCls == NULL) { \
             return; \
    }   \
    (*env)->ThrowNew(env, newExcCls, msg); \
  }



    /**
    * Liblucene Functions (module init, shutdown)
    */
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_initLibluceneJNI(JNIEnv *, 
                                                                         jobject, 
                                                                         jobject);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_shutdownLibluceneJNI(JNIEnv *, 
                                                                             jobject, 
                                                                             jobject);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_poolDestroy(JNIEnv *, 
                                                                jobject, 
                                                                jint);


    /**
    * IndexWriter functions
    */
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterCreateByPath(JNIEnv *, 
                                                                                jobject, 
                                                                                jobject, 
                                                                                jstring, 
                                                                                jboolean, 
                                                                                jint);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterAddDocument(JNIEnv *, jobject, jobject, jobject);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterClose(JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_indexWriterOptimize(JNIEnv *, jobject, jobject);


    /**
    * Document functions
    */
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_documentCreate(JNIEnv *, jobject, jobject, jint);
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_documentAddField(JNIEnv *, jobject, jobject, jobject, jint);
    JNIEXPORT void JNICALL
        Java_liblucene_liblucene4j_jni_LibluceneJNI_documentFree(JNIEnv *, jobject, jobject);

    /**
    * Field functions
    */
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldCreate(JNIEnv *, jobject, jobject, jstring, jstring, jint, jint);
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldBinaryCreate(JNIEnv *, jobject, jobject, jstring, jbyte[], jint, jint, jint);

    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_fieldSetAnalyzer(JNIEnv *, jobject, jobject, jobject);

    /**
    * Analyzer functions
    */
    JNIEXPORT void JNICALL 
        Java_liblucene_liblucene4j_jni_LibluceneJNI_germanAnalyzerCreate(JNIEnv *, jobject, jobject, jint);
#endif