// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include "host.h"
#include "org_blastdoor7_dvmc_EvmcVm.h"
#include <dvmc/helpers.h>
#include <dvmc/loader.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

JNIEXPORT jobject JNICALL Java_org_blastdoor7_dvmc_EvmcVm_load_1and_1create(JNIEnv* jenv,
                                                                          jclass jcls,
                                                                          jstring jfilename)
{
    (void)jcls;
    struct dvmc_vm* dvm = NULL;
    jint rs = dvmc_java_set_jvm(jenv);
    (void)rs;
    assert(rs == JNI_OK);
    // load the DVM
    const char* filename = (*jenv)->GetStringUTFChars(jenv, jfilename, NULL);
    assert(filename != NULL);
    enum dvmc_loader_error_code loader_error = DVMC_LOADER_UNSPECIFIED_ERROR;
    dvm = dvmc_load_and_create(filename, &loader_error);
    (*jenv)->ReleaseStringUTFChars(jenv, jfilename, filename);
    if (loader_error != DVMC_LOADER_SUCCESS)
    {
        const char* error_msg = dvmc_last_error_msg();
        jclass exception_class = (*jenv)->FindClass(jenv, "org/blastdoor7/dvmc/EvmcLoaderException");
        assert(exception_class != NULL);
        (*jenv)->ThrowNew(jenv, exception_class, error_msg ? error_msg : "Loading DVMC VM failed");
    }
    jobject jresult = (*jenv)->NewDirectByteBuffer(jenv, (void*)dvm, sizeof(struct dvmc_vm));
    assert(jresult != NULL);
    return jresult;
}

JNIEXPORT jint JNICALL Java_org_blastdoor7_dvmc_EvmcVm_abi_1version(JNIEnv* jenv, jclass jcls)
{
    (void)jenv;
    (void)jcls;
    return DVMC_ABI_VERSION;
}

JNIEXPORT jstring JNICALL Java_org_blastdoor7_dvmc_EvmcVm_name(JNIEnv* jenv,
                                                             jclass jcls,
                                                             jobject jdvm)
{
    (void)jcls;
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    const char* dvm_name = dvmc_vm_name(dvm);
    return (*jenv)->NewStringUTF(jenv, dvm_name);
}

JNIEXPORT jstring JNICALL Java_org_blastdoor7_dvmc_EvmcVm_version(JNIEnv* jenv,
                                                                jclass jcls,
                                                                jobject jdvm)
{
    (void)jcls;
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    const char* dvm_version = dvmc_vm_version(dvm);
    return (*jenv)->NewStringUTF(jenv, dvm_version);
}

JNIEXPORT void JNICALL Java_org_blastdoor7_dvmc_EvmcVm_destroy(JNIEnv* jenv,
                                                             jclass jcls,
                                                             jobject jdvm)
{
    (void)jcls;
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    dvmc_destroy(dvm);
}

static jobject AllocateDirect(JNIEnv* jenv, size_t capacity)
{
    const char java_class_name[] = "java/nio/ByteBuffer";
    const char java_method_name[] = "allocateDirect";
    const char java_method_signature[] = "(I)Ljava/nio/ByteBuffer;";

    jclass jcls = (*jenv)->FindClass(jenv, java_class_name);
    assert(jcls != NULL);
    jmethodID method =
        (*jenv)->GetStaticMethodID(jenv, jcls, java_method_name, java_method_signature);
    assert(method != NULL);
    return (*jenv)->CallStaticObjectMethod(jenv, jcls, method, capacity);
}

JNIEXPORT jobject JNICALL Java_org_blastdoor7_dvmc_EvmcVm_retrieve_desc_vx(JNIEnv* jenv,
                                                                jclass jcls,
                                                                jobject jdvm,
                                                                jobject jcontext,
                                                                jint jrev,
                                                                jobject jmsg,
                                                                jobject jcode)
{
    (void)jcls;
    struct dvmc_message* msg = (struct dvmc_message*)(*jenv)->GetDirectBufferAddress(jenv, jmsg);
    assert(msg != NULL);
    size_t code_size;
    const uint8_t* code = GetDirectBuffer(jenv, jcode, &code_size);
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    const struct dvmc_host_interface* host = dvmc_java_get_host_interface();
    jobject jresult = AllocateDirect(jenv, sizeof(struct dvmc_result));
    assert(jresult != NULL);
    struct dvmc_result* result =
        (struct dvmc_result*)(*jenv)->GetDirectBufferAddress(jenv, jresult);
    assert(result != NULL);
    *result = dvmc_retrieve_desc_vx(dvm, host, (struct dvmc_host_context*)jcontext, (enum dvmc_revision)jrev,
                           msg, code, code_size);
    return jresult;
}

JNIEXPORT jint JNICALL Java_org_blastdoor7_dvmc_EvmcVm_get_1capabilities(JNIEnv* jenv,
                                                                       jclass jcls,
                                                                       jobject jdvm)
{
    (void)jcls;
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    return (jint)dvm->get_capabilities(dvm);
}

JNIEXPORT jint JNICALL Java_org_blastdoor7_dvmc_EvmcVm_set_1option(JNIEnv* jenv,
                                                                 jclass jcls,
                                                                 jobject jdvm,
                                                                 jstring jname,
                                                                 jstring jval)
{
    (void)jcls;
    struct dvmc_vm* dvm = (struct dvmc_vm*)(*jenv)->GetDirectBufferAddress(jenv, jdvm);
    assert(dvm != NULL);
    const char* name = (*jenv)->GetStringUTFChars(jenv, jname, 0);
    const char* value = (*jenv)->GetStringUTFChars(jenv, jval, 0);
    assert(name != NULL);
    assert(value != NULL);
    enum dvmc_set_option_result option_result = dvmc_set_option(dvm, name, value);
    (*jenv)->ReleaseStringUTFChars(jenv, jname, name);
    (*jenv)->ReleaseStringUTFChars(jenv, jval, value);
    return (jint)option_result;
}
