#include <jni.h>
#include <string>
#include "DNFFmpeg.h"
DNFFmpeg *ffmpeg = 0;

JavaVM *javavm = 0;
int JNI_OnLoad(JavaVM *vm,void *r){
    javavm = vm;
    return JNI_VERSION_1_6;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1perpare(JNIEnv *env, jobject instance,
                                                         jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    JavaCallHelper *helper = new JavaCallHelper(javavm,env,instance);
    ffmpeg = new DNFFmpeg(helper,dataSource);
    ffmpeg->parpare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}