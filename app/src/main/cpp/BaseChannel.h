//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_BASECHANNEL_H
#define FFMPEGTEST_BASECHANNEL_H
extern "C" {
#include <libavcodec/avcodec.h>
};

#include "safe_queue.h"

class BaseChannel {
public:
    BaseChannel(int id,AVCodecContext* avCodecContext) : id(id),avCodecContext(avCodecContext) {}

    virtual ~BaseChannel() {
        packets.setReleaseHandle(BaseChannel::releaseAvPacket);
        packets.clear();
    }

    static void releaseAvPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }
    static void releaseAvFrame(AVFrame *&avFrame){
        if (avFrame) {
            av_frame_free(&avFrame);
            avFrame = 0;
        }
    }
    virtual void play() =0;

    int id;

    SafeQueue<AVPacket *> packets;
    bool isPlaying;
    AVCodecContext* avCodecContext;
};


#endif //FFMPEGTEST_BASECHANNEL_H
