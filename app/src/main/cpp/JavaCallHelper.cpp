//
// Created by hyc on 2018/9/19.
//

#include "JavaCallHelper.h"
#include "macro.h"
JavaCallHelper::JavaCallHelper(JavaVM *vm, JNIEnv *env, jobject instace) {
    this->vm = vm;
    //如果在主线程 回调
    this->env = env;
    this->instance = env->NewGlobalRef(instace);
    jclass  clazz = env->GetObjectClass(instace);
    onErrorId = env->GetMethodID(clazz,"onError","(I)V");
    onPrepareId = env->GetMethodID(clazz,"onPrepare","()V");
    onProgressId = env->GetMethodID(clazz,"onProgress","(I)V");
}
JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
}
void JavaCallHelper::onError(int thread, int errorCode) {
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorId,errorCode);
    }else
    {
        JNIEnv *env;
        if(vm->AttachCurrentThread(&env,0) != JNI_OK){
            return;
        }
        env->CallVoidMethod(instance,onErrorId,errorCode);
        vm->DetachCurrentThread();
    }
}
void JavaCallHelper::onPrepare(int thread) {
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onPrepareId);
    }else
    {
        JNIEnv *env;
        if(vm->AttachCurrentThread(&env,0) != JNI_OK){
            return;
        }
        env->CallVoidMethod(instance,onPrepareId);
        vm->DetachCurrentThread();
    }
}
void JavaCallHelper::onProgress(int thread, int progress) {
    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(instance, onProgressId, progress);
        vm->DetachCurrentThread();
    } else {
        env->CallVoidMethod(instance, onProgressId, progress);
    }
}