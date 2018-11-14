#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include "DNFFmpeg.h"

DNFFmpeg *ffmpeg = 0;

JavaVM *javavm = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int JNI_OnLoad(JavaVM *vm, void *r) {
    javavm = vm;
    return JNI_VERSION_1_6;
}

void render(uint8_t *data, int lineszie, int w, int h) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        return;
    }
    //设置窗口属性
    ANativeWindow_setBuffersGeometry(window, w,
                                     h,
                                     WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        return;
    }
//填充rgb数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4;
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, data + i * lineszie, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1perpare(JNIEnv *env, jobject instance,
                                                         jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    JavaCallHelper *helper = new JavaCallHelper(javavm, env, instance);
    ffmpeg = new DNFFmpeg(helper, dataSource);
    ffmpeg->setRenderFrameCallback(render);
    ffmpeg->parpare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1start(JNIEnv *env, jobject instance) {

    ffmpeg->start();

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1setSurface(JNIEnv *env, jobject instance,
                                                            jobject surface) {
    pthread_mutex_lock(&mutex);
    if (window) {
        //如果已经赋值了就要释放掉
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1stop(JNIEnv *env, jobject instance) {

    if (ffmpeg) {
        ffmpeg->stop();
      //  DELETE(ffmpeg);
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1release(JNIEnv *env, jobject instance) {

    if(window){
        ANativeWindow_release(window);
        window=0;
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1onResume(JNIEnv *env, jobject instance) {

    if(ffmpeg){


    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_hyc_ffmpegtest_DNPlayer_native_1GetDuration(JNIEnv *env, jobject instance) {

    if(ffmpeg){
        return ffmpeg->GetDUration();
    }
    return 0;

}