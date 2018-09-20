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
    this->dataSource = new char[strlen(dataSource)];
    strcpy(this->dataSource, dataSource);
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
    LOGE("asdddddddddd");
    formatContext = 0;
    int ret = avformat_open_input(&formatContext, dataSource, 0, 0);
    if (ret !=0) {
        LOGE("打开媒体失败:%s",av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        LOGE("查找流失败:%s",av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        AVCodecParameters *codecpar = stream ->codecpar;
        AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
        if(dec == NULL){
            LOGE("查找解码器失败:%s",av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        AVCodecContext *context = avcodec_alloc_context3(dec);
        if(context == NULL){
            LOGE("创建解码上下文失败:%s",av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        ret = avcodec_parameters_to_context(context,codecpar);
        if(ret < 0){
            LOGE("设置解码上下文参数失败:%s",av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        ret =  avcodec_open2(context,dec,0);
        if(ret != 0){
            LOGE("打开解码器失败:%s",av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoChannel = new VideoChannel;
        }else if(codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioChannel = new AudioChannel;
        }
    }
    //没有音视频（很少见）
    if(!audioChannel && !videoChannel){
        LOGE("没有音视频");
        callHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }
    LOGE("结束");
    callHelper->onPrepare(THREAD_CHILD);


};