#include "mainwindow.h"

#include <QApplication>

#include <SDL2/SDL.h>
#include "ffmpegutils.h"

//const int SCREEN_WIDTH = 640;
//const int SCREEN_HEIGHT = 480;
# define REFRESH_EVENT  (SDL_USEREVENT + 1) //刷新事件
# define BREAK_EVENT  (SDL_USEREVENT + 2) // 退出事件

int thread_exit = 0;
int thread_pause = 0;

//线程每40ms刷新一次
int video_refresh_thread(void *data) {
    thread_exit = 0;
    thread_pause = 0;

    while (!thread_exit) {
        if (!thread_pause) {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);// 发送刷新事件
        }
        SDL_Delay(40);
    }
    thread_exit = 0;
    thread_pause = 0;
    //Break
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;

    int ret = -1;
       std::string file = "/Users/autozi/Desktop/long_new_final.mp4";

       AVFormatContext *pFormatCtx = NULL;
       int  videoStream;
       AVCodecParameters *pCodecParameters = NULL;
       AVCodecContext *pCodecCtx = NULL;
       AVCodec *pCodec = NULL;
       AVFrame *pFrame = NULL;
       AVPacket packet;

       SDL_Rect rect;
       Uint32 pixformat;
       SDL_Window *win = NULL;
       SDL_Renderer *renderer = NULL;
       SDL_Texture *texture = NULL;
       SDL_Event event;
       //默认窗口大小
       int w_width = 640;
       int w_height = 480;

       //SDL初始化
       if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
           return ret;
       }


       // 打开输入文件
       if (avformat_open_input(&pFormatCtx, file.c_str(), NULL, NULL) != 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open  video file!");
           goto __FAIL;
       }

       //找到视频流
       videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
       if (videoStream == -1) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Din't find a video stream!");
           goto __FAIL;// Didn't find a video stream
       }

       // 流参数
       pCodecParameters = pFormatCtx->streams[videoStream]->codecpar;

       //获取解码器
       pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
       if (pCodec == NULL) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec!\n");
           goto __FAIL; // Codec not found
       }

       // 初始化一个编解码上下文
       pCodecCtx = avcodec_alloc_context3(pCodec);
       if (avcodec_parameters_to_context(pCodecCtx, pCodecParameters) != 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't copy codec context");
           goto __FAIL;// Error copying codec context
       }

       // 打开解码器
       if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open decoder!\n");
           goto __FAIL; // Could not open codec
       }

       // Allocate video frame
       pFrame = av_frame_alloc();

       w_width = pCodecCtx->width;
       w_height = pCodecCtx->height;

       //创建窗口
       win = SDL_CreateWindow("Media Player",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w_width, w_height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
       if (!win) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window by SDL");
           goto __FAIL;
       }

       //创建渲染器
       renderer = SDL_CreateRenderer(win, -1, 0);
       if (!renderer) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Renderer by SDL");
           goto __FAIL;
       }

       pixformat = SDL_PIXELFORMAT_IYUV;//YUV格式
       // 创建纹理
       texture = SDL_CreateTexture(renderer,
                                   pixformat,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   w_width,
                                   w_height);


       SDL_CreateThread(video_refresh_thread, "Video Thread", NULL);
       //读取数据
       for (;;) {
           SDL_WaitEvent(&event);//使用事件驱动，每40ms执行一次
           if (event.type == REFRESH_EVENT) {
               while (1) {
                   if (av_read_frame(pFormatCtx, &packet) < 0)
                       thread_exit = 1;

                   if (packet.stream_index == videoStream)
                       break;
               }

               if (packet.stream_index == videoStream) {
                   avcodec_send_packet(pCodecCtx, &packet);
                   while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {

                       SDL_UpdateYUVTexture(texture, NULL,
                                            pFrame->data[0], pFrame->linesize[0],
                                            pFrame->data[1], pFrame->linesize[1],
                                            pFrame->data[2], pFrame->linesize[2]);

                       // Set Size of Window
                       rect.x = 0;
                       rect.y = 0;
                       rect.w = pCodecCtx->width;
                       rect.h = pCodecCtx->height;

                       SDL_RenderClear(renderer);
                       SDL_RenderCopy(renderer, texture, NULL, &rect);
                       SDL_RenderPresent(renderer);
                   }
                   av_packet_unref(&packet);
               }
           } else if (event.type == SDL_KEYDOWN) {
               if (event.key.keysym.sym == SDLK_SPACE) { //空格键暂停
                   thread_pause = !thread_pause;
               }
               if (event.key.keysym.sym== SDLK_ESCAPE){ // ESC键退出
                   thread_exit = 1;
               }
           } else if (event.type == SDL_QUIT) {
               thread_exit = 1;
           } else if (event.type == BREAK_EVENT) {
               break;
           }
       }

       __FAIL:
       // Free the YUV frame
       if (pFrame) {
           av_frame_free(&pFrame);
       }

       // Close the codec
       if (pCodecCtx) {
           avcodec_close(pCodecCtx);
       }

       if (pCodecParameters) {
           avcodec_parameters_free(&pCodecParameters);
       }

       // Close the video file
       if (pFormatCtx) {
           avformat_close_input(&pFormatCtx);
       }

       if (win) {
           SDL_DestroyWindow(win);
       }

       if (renderer) {
           SDL_DestroyRenderer(renderer);
       }

       if (texture) {
           SDL_DestroyTexture(texture);
       }

       SDL_Quit();

       return ret;


//    w.show();
//    return a.exec();
}
