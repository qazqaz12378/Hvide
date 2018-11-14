//
// Created by hyc on 2018/9/20.
//

#ifndef FFMPEGTEST_BASECHANNEL_H
#define FFMPEGTEST_BASECHANNEL_H
extern "C" {
#include <libavcodec/avcodec.h>
};

#include "safe_queue.h"
#include "JavaCallHelper.h"

class BaseChannel {
public:
    BaseChannel(int id,JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                AVRational base) : id(id),
                                   javaCallHelper(javaCallHelper),
                                   avCodecContext(
            avCodecContext), time_base(base) {}

    virtual ~BaseChannel() {
        pkt_queue.setReleaseHandle(BaseChannel::releaseAvPacket);
        pkt_queue.clear();
    }

    static void releaseAvPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }

    static void releaseAvFrame(AVFrame *&avFrame) {
        if (avFrame) {
            av_frame_free(&avFrame);
            avFrame = 0;
        }
    }

    void clear() {
        pkt_queue.clear();
        frame_queue.clear();
    }

    void stopWork() {
        pkt_queue.setWork(0);
        frame_queue.setWork(0);
    }

    void startWork() {
        pkt_queue.setWork(1);
        frame_queue.setWork(1);
    }

    virtual void play() = 0;

    virtual void stop() = 0;

    int id;
    double clock = 0;
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile bool isPlaying = false;
    volatile int channelId;
    AVCodecContext *avCodecContext;
    AVRational time_base;
    JavaCallHelper *javaCallHelper;
};


#endif //FFMPEGTEST_BASECHANNEL_H
