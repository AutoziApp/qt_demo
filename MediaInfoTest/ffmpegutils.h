#ifndef FFMPEGUTILS_H
#define FFMPEGUTILS_H
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
#include <libavutil/log.h>
}

class FFmpegUtils
{
public:
    FFmpegUtils();
};

#endif // FFMPEGUTILS_H
