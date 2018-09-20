//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_AUDIOCHANNEL_H
#define FFMPEGTEST_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext* avCodecContext);
    ~AudioChannel();
    void play();
    void decode();
    void render();
};


#endif //FFMPEGTEST_AUDIOCHANNEL_H
