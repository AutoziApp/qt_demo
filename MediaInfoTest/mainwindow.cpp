#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QWidget>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , strLocalPath("")
    , mPlayWindow(NULL)
    , mRenderer(NULL)
    , mTexture(NULL)
    , mpFormatCtx(NULL)
    , mpCodecParam(NULL)
    , mpCodecCtx(NULL)
    , mpCodec(NULL)

{
    ui->setupUi(this);

    this->initSDL();
}

MainWindow::~MainWindow()
{
    delete ui;
}
//初始化SDL
void MainWindow::initSDL()
{
    int flags  = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
    if (SDL_Init (flags)) {
           av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
           av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
           return ;
       }

    av_log(NULL, AV_LOG_INFO, "(SDL initialize finished.)\n");
}

void MainWindow::setupWindowParam()
{
    int flags = SDL_WINDOW_ALLOW_HIGHDPI;
#if SDL_VERSION_ATLEAST(2,0,5)
            flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#else
            av_log(NULL, AV_LOG_WARNING, "Your SDL version doesn't support SDL_WINDOW_ALWAYS_ON_TOP. Feature will be inactive.\n");
#endif
    flags |= SDL_WINDOW_RESIZABLE;
    if (mPlayWindow == NULL) {
        mPlayWindow = SDL_CreateWindow("PlayWindow",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       mDefineWidth,
                                       mDefineHeight,
                                       flags);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    }
    if(mPlayWindow != NULL){

        //创建渲染器
        mRenderer = SDL_CreateRenderer(mPlayWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!mRenderer) {
            av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
            mRenderer = SDL_CreateRenderer(mPlayWindow, -1, 0);
        }
        if (mRenderer) {
            if (!SDL_GetRendererInfo(mRenderer, &mRendererInfo))
                av_log(NULL, AV_LOG_INFO, "Initialized %s renderer.\n", mRendererInfo.name);
        }
    }
    // 创建纹理
    if(mTexture ==NULL) {
        mTexture = SDL_CreateTexture(mRenderer,
                                    SDL_PIXELFORMAT_IYUV,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    mDefineWidth,
                                    mDefineHeight);
    }
    if (!mPlayWindow || !mRenderer || !mRendererInfo.num_texture_formats || !mTexture) {
        av_log(NULL, AV_LOG_FATAL, "Failed to create window renderer or texture: %s", SDL_GetError());
        return;
    }
}

// FFmpge
void MainWindow::initFFmpeg() {
    if (avformat_open_input(&mpFormatCtx, sLocalPath.c_str(), NULL, NULL) != 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open  video file!");
           return ;
       }

       //找到视频流
      int  videoStream = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
       if (videoStream == -1) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Din't find a video stream!");
           return ;
       }

       // 流参数
       mpCodecParam = mpFormatCtx->streams[videoStream]->codecpar;

       //获取解码器
       mpCodec = avcodec_find_decoder(mpCodecParam->codec_id);
       if (mpCodec == NULL) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec!\n");
           return ;
       }

       // 初始化一个编解码上下文
       mpCodecCtx = avcodec_alloc_context3(mpCodec);
       if (avcodec_parameters_to_context(mpCodecCtx, mpCodecParam) != 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't copy codec context");
           return ;// Error copying codec context
       }

       // 打开解码器
       if (avcodec_open2(mpCodecCtx, mpCodec, NULL) < 0) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open decoder!\n");
           return ; // Could not open codec
       }

       // Allocate video frame
       mpFrame = av_frame_alloc();

       this->mDefineWidth = mpCodecCtx->width;
       this->mDefineHeight = mpCodecCtx->height;

       //读取数据
           while (av_read_frame(mpFormatCtx, &mpPacket) >= 0) {
               if (mpPacket.stream_index == videoStream) {
                   //解码
                   avcodec_send_packet(mpCodecCtx, &mpPacket);
                   while (avcodec_receive_frame(mpCodecCtx, mpFrame) == 0) {

                       SDL_UpdateYUVTexture(mTexture, NULL,
                                            mpFrame->data[0], mpFrame->linesize[0],
                                            mpFrame->data[1], mpFrame->linesize[1],
                                            mpFrame->data[2], mpFrame->linesize[2]);

                       // Set Size of Window
                       mRect.x = 0;
                       mRect.y = 0;
                       mRect.w = mpCodecCtx->width;
                       mRect.h = mpCodecCtx->height;

                       //展示
                       SDL_RenderClear(mRenderer);
                       SDL_RenderCopy(mRenderer, mTexture, NULL, &mRect);
                       SDL_RenderPresent(mRenderer);
                   }
               }

               av_packet_unref(&mpPacket);

               // 事件处理
               SDL_Event event;
               SDL_PollEvent(&event);
               switch (event.type) {
                   case SDL_QUIT:
                        break;
//                       goto __QUIT;
//                   default:
//                       break;
               }
           }
}
//若当前对象cmbLocalPath值发生改变则触发此函数
void MainWindow::on_cmbLocalPath_currentIndexChanged(const QString &arg1)
{
//    this->strLocalPath = ui->cmbLocalPath->currentText();
    this->strLocalPath = arg1;
    this->sLocalPath = arg1.toStdString();
   qDebug()<<"hello "<<strLocalPath<<endl;
}

void MainWindow::on_btnOpenFile_clicked()
{
    QFileDialog *fileDlg = new QFileDialog(this);
    fileDlg->setWindowTitle(tr("请选择视频文件"));
    fileDlg->setDirectory(".");
    QStringList filters;
    filters << "Video files (*.h264 *.mp4 *.ts *.yuv)"
            << "Any files (*)";
    fileDlg->setNameFilters(filters);
    fileDlg->setViewMode(QFileDialog::Detail);
    if(fileDlg->exec()== QDialog::Rejected) {
        return ;
    }

    QString sPathName = fileDlg->selectedFiles()[0];
    if (sPathName.isEmpty())
    {
        return ;
    }
    ui->cmbLocalPath->setCurrentText(sPathName);
    int index  = ui->cmbLocalPath->findText(sPathName);
    if(index < 0)
        ui->cmbLocalPath->insertItem(0, sPathName);

}
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)


void MainWindow::on_btnPlay_clicked()
{
    initFFmpeg();

//    SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,this);
}


