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
private:
    char *dataSource;
    pthread_t pid;
    AVFormatContext *formatContext;
    JavaCallHelper* callHelper;
    AudioChannel *audioChannel;
    VideoChannel *videoChannel;
};


#endif //FFMPEGTEST_DNFFMPEG_H
