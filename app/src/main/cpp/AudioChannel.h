//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_AUDIOCHANNEL_H
#define FFMPEGTEST_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES_Android.h>
#include "macro.h"

extern "C" {
#include <libswresample/swresample.h>

}

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id,JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,AVRational base);

    ~AudioChannel();

    virtual void play();

    virtual void stop();

    void decode();

    void render();

    void initOpenSL();
    int decodePcm();
    int getPcm();
    uint8_t *buffer;
private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;

    //opensl es
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;
    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

    SLObjectItf bqplayerObject = NULL;
    SLPlayItf bqPlayerPlay = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;

    SwrContext *swr_ctx = NULL;
    int out_channels;
    int out_samplesize;


};


#endif //FFMPEGTEST_AUDIOCHANNEL_H
