//
// Created by hyc on 2018/9/19.
//

#ifndef FFMPEGTEST_JAVACALLHELPER_H
#define FFMPEGTEST_JAVACALLHELPER_H


#include <jni.h>

class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *vm,JNIEnv* env,jobject instace);
    ~JavaCallHelper();
    void onError(int thread,int errorCode);
    void onPrepare(int thread);
private:
    JavaVM *vm;
    JNIEnv *env;
    jobject  instance;
    jmethodID onErrorId;
    jmethodID onPrepareId;
};


#endif //FFMPEGTEST_JAVACALLHELPER_H
