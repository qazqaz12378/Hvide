//
// Created by hyc on 2018/9/19.
//
#include <cstring>
#include <pthread.h>
#include "DNFFmpeg.h"
#include "macro.h"

void *task_prepare(void *args) {
    DNFFmpeg *ffmpeg = static_cast<DNFFmpeg *>(args);
    ffmpeg->_parpare();
    return 0;

}

DNFFmpeg::DNFFmpeg(JavaCallHelper *callHelper, const char *dataSource) {
    this->callHelper = callHelper;
    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);
    isPlaying = false;
    //duration = 0;
}

DNFFmpeg::~DNFFmpeg() {
    DELETE(dataSource);

    DELETE(callHelper)
}

void DNFFmpeg::parpare() {
    pthread_create(&pid, 0, task_prepare, this);
}

void DNFFmpeg::_parpare() {
    //初始化网络
    avformat_network_init();
    AVDictionary *opts = NULL;
    av_dict_set(&opts,"timeout","5000000",0);
    int ret = avformat_open_input(&formatContext, dataSource, NULL, &opts);
    if (ret != 0) {
        LOGE("打开媒体失败:%s", av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        LOGE("查找流失败:%s", av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    duration = formatContext->duration / 1000000;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
        if (dec == NULL) {
            LOGE("查找解码器失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        AVCodecContext *context = avcodec_alloc_context3(dec);
        if (context == NULL) {
            LOGE("创建解码上下文失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        ret = avcodec_parameters_to_context(context, codecpar);
        if (ret < 0) {
            LOGE("设置解码上下文参数失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        ret = avcodec_open2(context, dec, 0);
        if (ret != 0) {
            LOGE("打开解码器失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        AVRational base = formatContext->streams[i]->time_base;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            int FPS = av_q2d(formatContext->streams[i]->avg_frame_rate);
            videoChannel = new VideoChannel(i, callHelper,context,base,FPS);
            videoChannel->setRenderFrameCallback(callback);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i,callHelper, context,base);
        }
    }
    //没有音视频（很少见）
    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        callHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }
    LOGE("结束");
    callHelper->onPrepare(THREAD_CHILD);


};

void *play(void *args) {
    DNFFmpeg *ffmpeg = static_cast<DNFFmpeg *>(args);
    ffmpeg->_start();
    return 0;
}

void DNFFmpeg::start() {
    isPlaying = true;
    if (videoChannel) {
        videoChannel->audioChannel = audioChannel;
        videoChannel->play();
    }
    if(audioChannel){
        audioChannel->play();
    }
    pthread_create(&pid_play, 0, play, this);
}

void DNFFmpeg::_start() {
    int ret = 0;
    while (isPlaying) {
        LOGE("数据 %d:",formatContext->pb->pos);
        if(audioChannel && audioChannel->pkt_queue.size() > 100){
            LOGE("audio 积压.");
            av_usleep(1000 * 10);
            continue;
        }else if(videoChannel && videoChannel->pkt_queue.size() > 100){
            LOGE("video 积压..");
            av_usleep(1000 * 10);
            continue;
        }

       // pthread_mutex_lock(&seekMutex);

        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext, packet);
        //pthread_mutex_unlock(&seekMutex);
        if (ret == 0) {
            if (audioChannel && packet->stream_index == audioChannel->id) {
                audioChannel->pkt_queue.enQueue(packet);
            } else if (videoChannel && packet->stream_index == videoChannel->id) {
                videoChannel->pkt_queue.enQueue(packet);
            }
        } else if (ret == AVERROR_EOF) {

        } else {

        }
    }
    isPlaying = 0;
    audioChannel->stop();
    videoChannel->stop();
}

void DNFFmpeg::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}
void DNFFmpeg::stop() {
    pthread_join(pid,0);
    isPlaying = 0;
    pthread_join(pid_play,0);
}
void DNFFmpeg::Resume(){
    pthread_join(pid,0);
    isPlaying = 1;
    pthread_join(pid_play,0);
}