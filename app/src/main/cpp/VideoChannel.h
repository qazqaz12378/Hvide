//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_VIDEOCHANNEL_H
#define FFMPEGTEST_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"

typedef void (*RenderFrameCallback)(uint8_t *, int,int , int);
class VideoChannel: public BaseChannel {
public:
    VideoChannel(int id,JavaCallHelper *javaCallHelper,AVCodecContext* avCodecContext,AVRational base,int fps);
    ~VideoChannel();

    virtual void play();

    virtual void stop();
    void decode();
    void render();
    void setRenderFrameCallback(RenderFrameCallback callback);

public:
    AudioChannel *audioChannel = 0;
private:
    pthread_t pid_decode;
    pthread_t  pid_render;

    RenderFrameCallback callback;
    int fps;
};


#endif //FFMPEGTEST_VIDEOCHANNEL_H
