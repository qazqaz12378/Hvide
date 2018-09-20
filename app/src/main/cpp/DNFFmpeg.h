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
    AVFormatContext *formatContext;
    JavaCallHelper* callHelper;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    RenderFrameCallback callback;
    bool isPlaying;
};


#endif //FFMPEGTEST_DNFFMPEG_H
