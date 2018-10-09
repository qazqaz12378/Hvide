//
// Created by hyc on 2018/9/19.
//

#ifndef FFMPEGTEST_DNFFMPEG_H
#define FFMPEGTEST_DNFFMPEG_H

#include "JavaCallHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}
class DNFFmpeg {
public:
    DNFFmpeg(JavaCallHelper* callHelper,const char* dataSource);
    ~DNFFmpeg();
    void parpare();
    void _parpare();
    void start();
    void _start();
    void setRenderFrameCallback(RenderFrameCallback callback);
private:
    char *dataSource;
    pthread_t pid;
    pthread_t  pid_play;
    AVFormatContext *formatContext = 0;
    JavaCallHelper* callHelper;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    RenderFrameCallback callback;
    bool isPlaying;
    pthread_mutex_t seekMutex;
};


#endif //FFMPEGTEST_DNFFMPEG_H
