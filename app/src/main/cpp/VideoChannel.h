//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_VIDEOCHANNEL_H
#define FFMPEGTEST_VIDEOCHANNEL_H


#include "BaseChannel.h"
typedef void (*RenderFrameCallback)(uint8_t *, int,int , int);
class VideoChannel: public BaseChannel {
public:
    VideoChannel(int id,AVCodecContext* avCodecContext);
    ~VideoChannel();
    void play();
    void decode();
    void render();
    void setRenderFrameCallback(RenderFrameCallback callback);
private:
    pthread_t pid_decode;
    pthread_t  pid_render;
    SafeQueue<AVFrame*> frames;
    RenderFrameCallback callback;
};


#endif //FFMPEGTEST_VIDEOCHANNEL_H
