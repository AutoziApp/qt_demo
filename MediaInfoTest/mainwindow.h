#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sdl2utils.h"
#include "ffmpegutils.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initSDL();
    void setupWindowParam();
    void  initFFmpeg();


private slots:
    void on_cmbLocalPath_currentIndexChanged(const QString &arg1);

    void on_btnOpenFile_clicked();

    void on_btnPlay_clicked();

private:
    Ui::MainWindow *ui;
public:
    //本地路径
    QString strLocalPath;
    std::string sLocalPath;

    SDL_Window *mPlayWindow;
    SDL_Renderer *mRenderer;
    SDL_RendererInfo mRendererInfo;
    SDL_Texture *mTexture;
    int mDefineWidth;
    int mDefineHeight;
    SDL_Rect mRect;
    SDL_Event event;
    AVFormatContext *mpFormatCtx;
    AVCodecParameters *mpCodecParam;
    AVCodecContext  *mpCodecCtx;
    AVCodec *mpCodec;
    AVFrame *mpFrame;
    AVPacket mpPacket;

    int thread_exit;
    int thread_pause;


};
#endif // MAINWINDOW_H
