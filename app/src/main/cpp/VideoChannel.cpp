//
// Created by hyc on 2018/9/20.
//




#include "VideoChannel.h"


extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
SwsContext *swsContext;
void *decode_task(void *args) {
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    return 0;
}

void *render_task(void *args) {
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->render();
    return 0;
}

VideoChannel::VideoChannel(int id, AVCodecContext *avCodecContext) : BaseChannel(id,
                                                                                 avCodecContext) {
    frames.setReleaseHandle(releaseAvFrame);
}

VideoChannel::~VideoChannel() {
    frames.clear();
}

void VideoChannel::play() {
    isPlaying = 1;
    pthread_create(&pid_decode, 0, decode_task, this);
    pthread_create(&pid_render, 0, render_task, this);
}

void VideoChannel::decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = packets.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret != 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            break;
        }
        frames.enQueue(frame);
    }
    releaseAvPacket(packet);
}

void VideoChannel::render() {
    swsContext = sws_getContext(avCodecContext->width,
                                avCodecContext->height, avCodecContext->pix_fmt,
                                avCodecContext->width,
                                avCodecContext->height,
                                AV_PIX_FMT_RGBA, SWS_BILINEAR, 0, 0, 0);
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    AVFrame *frame = 0;
    while (isPlaying) {
        frames.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *> (frame->data),
                  frame->linesize, 0, avCodecContext->height, dst_data, dst_linesize);
        callback(dst_data[0], dst_linesize[0], avCodecContext->width,
                 avCodecContext->height);
        releaseAvFrame(frame);
    }
    av_free(&dst_data[0]);
    releaseAvFrame(frame);
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}