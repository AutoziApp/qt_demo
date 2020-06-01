#include <QCoreApplication>
#include <SDL2/SDL.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) == -1) {
        qDebug("Could not initialize SDL: %s.\n", SDL_GetError());
        return -1;
    } else {
        qDebug("SDL initialize ok!");
    }
    SDL_Window *win = nullptr;
    // 创建一个窗口， 并返回一个SDL_Window 指针，
    // params： 窗口标题
    // x, y 的坐标，
    // 窗口的大小w，h
    // params:  窗口的各种flag
    win = SDL_CreateWindow("Hello World！", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if(win == nullptr) {
        qDebug("Create window error: %s.\n", SDL_GetError());
        return -1;
    }
   /*
    * 把东西绘制到窗口，使用函数 SDL_CreateRenderer创建渲染器，
    * params： 要绘制的窗口
    * params: 显卡驱动，-1表示自动选择合适我们选项的驱动。
    * SDL_RENDERER_ACCELERATED  我们想使用硬件加速渲染器即利用显卡的力量，
    * SDL_RENDERER_PRESENTVSYNC 以显示器的刷新率来更新画面
    */
    SDL_Renderer* render= nullptr;
    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (render == nullptr ) {
        qDebug("Create Render error: %s.\n", SDL_GetError());
        return -1;
    }
    /*
     * 使用SDL_Texture 来用硬件加速绘制图像，然后用SDL_LoadBMP函数将图片加载到SDL_Surface中，
     */
    SDL_Surface *bmp = nullptr;
    bmp = SDL_LoadBMP("/Users/autozi/Desktop/testyuv.bmp");
    if (bmp == nullptr) {
        qDebug("Load bmp error: %s.\n", SDL_GetError());
        return -1;
    }
    /**
     * @brief
     * 要有效的利用硬件加速来绘制，我们必须吧SDL_Surface转换为SDL_Texture,这样renderer才能够绘制
     */
    SDL_Texture *tex = nullptr;
    tex = SDL_CreateTextureFromSurface(render, bmp);
    SDL_FreeSurface(bmp);
    /**
     * @brief getchar
     * 使用SDL_RenderPresent来跟新屏幕的画面
     */
    SDL_RenderClear(render);
    // 使用SDL_RenderCopy 传两个NULL， 第一个事指向原矩形的指针，第二个NULL是目标矩形的指针。
    // null 表示填满整个窗口。

    SDL_RenderCopy(render, tex, NULL, NULL);
    SDL_RenderPresent(render);
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = true;
                qDebug("SDL finished...\n");
            }
        }
    }
//    SDL_Delay(20000);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return a.exec();
}










