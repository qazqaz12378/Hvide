//
// Created by hyc on 2018/9/20.
//

extern "C" {
#include <libavutil/time.h>
}
#include "AudioChannel.h"


void *audioPlay(void *args){
    AudioChannel *audio = static_cast<AudioChannel *>(args);
    audio->initOpenSL();
    return 0;
}
void *audioDecode(void *args){
    AudioChannel *audio = static_cast<AudioChannel *>(args);
    audio->decode();
    return 0;
}
AudioChannel::AudioChannel(int id, AVCodecContext *avCodecContext,AVRational base) : BaseChannel(id,
                                                                                 avCodecContext,base) {
    //根据布局获取声道数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

    //CD音频标准
    //44100 双声道 2字节
    buffer = (uint8_t *) malloc(44100 * out_samplesize * out_channels);
}

AudioChannel::~AudioChannel() {
    free(buffer);
    buffer =0;
}

void AudioChannel::play() {
//重采样为固定的双声道 16位 44100
    swr_ctx = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                 avCodecContext->channel_layout,
                                 avCodecContext->sample_fmt,
                                 avCodecContext->sample_rate, 0, 0);
    swr_init(swr_ctx);

    startWork();
    isPlaying = true;
    pthread_create(&pid_audio_play, NULL, audioPlay, this);
    pthread_create(&pid_audio_decode,NULL,audioDecode,this);
}
void AudioChannel::stop() {

}
void bqPlayerCalback(SLAndroidSimpleBufferQueueItf bq, void *context){
    AudioChannel *audioChannel =static_cast<AudioChannel *>(context);
    int datalen = audioChannel->getPcm();
    if(datalen > 0){
        (*bq)->Enqueue(bq,audioChannel->buffer,datalen);
    }
}
void AudioChannel::initOpenSL() {
    //创建引擎
    SLresult result;

    //create engine

    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("音频初始化失败");
        return;
    }
    //realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("音频创建失败");
        return;
    }
    //get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("获取引擎接口失败");
        return;
    }

    // 创建混音器outputMixObject
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0,
                                              0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        return;
    }
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if(SL_RESULT_SUCCESS == result){
        const SLEnvironmentalReverbSettings reverbSettings =
                SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }
    //  创建队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&android_queue,&format_pcm};
    // configure audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //创建播放器
    result = (*engineEngine)->CreateAudioPlayer(engineEngine,&bqplayerObject,&audioSrc,&audioSnk,1,ids,req);
    //初始化播放器
    result = (*bqplayerObject)->Realize(bqplayerObject,SL_BOOLEAN_FALSE);
    (*bqplayerObject)->GetInterface(bqplayerObject,SL_IID_PLAY,&bqPlayerPlay);
    //    得到接口后调用  获取Player接口
    (*bqplayerObject)->GetInterface(bqplayerObject, SL_IID_PLAY, &bqPlayerPlay);

//    注册回调缓冲区 //获取缓冲队列接口
    (*bqplayerObject)->GetInterface(bqplayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCalback, this);
//    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCalback(bqPlayerBufferQueue, this);
}
int AudioChannel::decodePcm() {
    AVPacket *packet = 0;
    AVFrame *frame = av_frame_alloc();
    int data_size = 0;
    while (isPlaying){
        int ret = pkt_queue.deQueue(packet);
        if(!isPlaying){
            break;
        }
        if(!ret){
            continue;
        }
        ret = avcodec_send_packet(avCodecContext,packet);
        releaseAvPacket(packet);
        if(ret == AVERROR(EAGAIN)){
            continue;
        } else if(ret < 0){
            break;
        }
        ret = avcodec_receive_frame(avCodecContext,frame);
        if(ret == AVERROR(EAGAIN)){
            continue;
        }else if(ret < 0){
            break;
        }
        // 计算转换后的sample个数 a * b / c
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                frame->sample_rate,
                frame->sample_rate, AV_ROUND_UP);

        // 转换，返回值为转换后的sample个数
        int nb = swr_convert(swr_ctx, &buffer, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        //转换后多少数据
        data_size = nb * out_channels * out_samplesize;
        //音频的时间
        clock = frame->best_effort_timestamp * av_q2d(time_base);
        break;
    }
    releaseAvFrame(frame);
    releaseAvPacket(packet);
    return data_size;
}
void AudioChannel::decode() {
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
            //需要更多数据
            continue;
        } else if (ret < 0) {
            //失败
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
            continue;
        }
        frame_queue.enQueue(frame);
    }
    releaseAvPacket(packet);
}
int AudioChannel::getPcm() {
    int data_size = 0;
    AVFrame *frame = 0;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        // 计算转换后的sample个数  类似 a * b / c
        // swr_get_delay： 延迟时间:  输入了10个 数据，可能这次转换只转换了8个数据，那么还剩余2个 这一个函数就是得到上一次剩余的这个2
        // av_rescale_rnd： 以3为单位的1 转为以2为单位
        // 10个2=20
        // 转成 以4 为单位
        // 10*4/2
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                44100,
                frame->sample_rate,
                AV_ROUND_UP);
        // 转换，返回值为转换后的sample个数
        int nb = swr_convert(swr_ctx, &buffer, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        //转换后多少数据
        data_size = nb * out_channels * out_samplesize;
        //音频的时间
        clock = frame->best_effort_timestamp * av_q2d(time_base);
        if (javaCallHelper) {
            javaCallHelper->onProgress(THREAD_CHILD, clock);
        }
        break;
    }
    releaseAvFrame(frame);
    return data_size;
}