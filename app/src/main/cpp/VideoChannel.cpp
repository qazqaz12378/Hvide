//
// Created by hyc on 2018/9/20.
//




#include "VideoChannel.h"


extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
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
void dropFrame(queue<AVFrame *> &q){
    if(!q.empty()){
        AVFrame *frame = q.front();
        q.pop();
        BaseChannel::releaseAvFrame(frame);
    }
}
void dropPacket(queue<AVPacket *> &q){
    while(!q.empty()){
        AVPacket *pkt = q.front();
        if(pkt->flags != AV_PKT_FLAG_KEY){
            q.pop();
            BaseChannel::releaseAvPacket(pkt);
        } else{
            break;
        }
    }
}
VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper,AVCodecContext *avCodecContext,AVRational base,int fps) : BaseChannel(id,javaCallHelper,
                                                                                 avCodecContext,base),fps(fps) {
    frame_queue.setReleaseHandle(releaseAvFrame);
    frame_queue.setSyncHandle(dropFrame);
}

VideoChannel::~VideoChannel() {
        frame_queue.clear();
}

void VideoChannel::play() {

    startWork();
    isPlaying = 1;
    pthread_create(&pid_decode, 0, decode_task, this);
    pthread_create(&pid_render, 0, render_task, this);
}
void VideoChannel::stop() {
    isPlaying =0;
}

void VideoChannel::decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }else if(ret < 0){
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            break;
        }
        frame_queue.enQueue(frame);
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
    double frame_delay = 1.0 / fps;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        if(!ret){
            LOGE("没有取出来");
            continue;
        }
#if 1
        //显示时间戳 什么时候显示这个frame
        if((clock = frame->best_effort_timestamp) == AV_NOPTS_VALUE){
            clock = 0;
        }
        //pts 单位就是time_base
        //av_q2d转为双精度浮点数 乘以 pts 得到pts --- 显示时间:秒
        clock = clock * av_q2d(time_base);
        //frame->repeat_pict = 当解码时，这张图片需要要延迟多久显示
        //需要求出扩展延时：
        //extra_delay = repeat_pict / (2*fps) 需要延迟这么久来显示
        double repeat_pict = frame->repeat_pict;
        double extra_delay = repeat_pict / (2 * fps);

        double delay = extra_delay + frame_delay;

        if(clock == 0) {
            av_usleep(delay * 1000000);

        } else{
            double audioClock = audioChannel ? audioChannel->clock : 0;
            double diff = fabs(clock - audioClock);
            //LOGE("当前和音频比较:%f - %f = %f", clock, audioClock, diff);
            if (audioChannel) {
                //如果视频比音频快，延迟差值播放，否则直接播放
                if (clock > audioClock) {
                    if (diff > 1) {
                        //差的太久了， 那只能慢慢赶 不然就是卡好久
                        av_usleep((delay * 2) * 1000000);
                    } else {
                        //差的不多，尝试一次赶上去
                        av_usleep((delay + diff) * 1000000);
                    }
                } else {
                    //音频比视频快
                    //视频慢了 0.05s 已经比较明显了 (丢帧)
                    if (diff > 1) {
                        //一种可能： 快进了(因为解码器中有缓存数据，这样获得的avframe就和seek的匹配了)
                    }else if(diff >= 0.05){
                        releaseAvFrame(frame);
                        frame_queue.sync();
                    } else {
                        //不休眠 加快速度赶上去
                    }
                }
            } else {
                //正常播放
                av_usleep(delay * 1000000);
            }
        }

#endif
        if(javaCallHelper && !audioChannel){
            javaCallHelper->onProgress(THREAD_CHILD,clock);
        }

        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *> (frame->data),
                  frame->linesize, 0, avCodecContext->height, dst_data, dst_linesize);
        callback(dst_data[0], dst_linesize[0], avCodecContext->width,
                 avCodecContext->height);
        releaseAvFrame(frame);
    }
    av_free(&dst_data[0]);
    releaseAvFrame(frame);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}