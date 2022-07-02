// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#include "dvmc/dvmc.h"
#include <assert.h>
#include <jni.h>

#ifndef _Included_org_blastdoor7_dvmc_Host
#define _Included_org_blastdoor7_dvmc_Host
#ifdef __cplusplus
extern "C" {
#endif

int dvmc_java_set_jvm(JNIEnv*);
const struct dvmc_host_interface* dvmc_java_get_host_interface();

static inline void* GetDirectBuffer(JNIEnv* jenv, jobject buf, size_t* size)
{
    void* ret = (uint8_t*)(*jenv)->GetDirectBufferAddress(jenv, buf);
    assert(ret != NULL);
    jlong buf_size = (*jenv)->GetDirectBufferCapacity(jenv, buf);
    assert(buf_size != -1);
    if (size)
        *size = (size_t)buf_size;
    return ret;
}

#ifdef __cplusplus
}
#endif
#endif
