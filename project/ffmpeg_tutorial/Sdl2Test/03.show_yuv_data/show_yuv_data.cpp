#include <iostream>

#include <SDL.h>

using namespace std;

const int WIDTH = 1000, HEIGHT = 667;

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("Hello SDL world!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (NULL == window) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return -1;
    }

    /**
     * 从文件中提取YUV?
     *
     * 关键在于YUV数据的格式和size。格式决定了YUV分量在文件中的排列方式，size决定了总的像素值。我们已经知道图片是I420格式。也就是yuv420p，它是三平面存储，每个像素内存计算方式是：
     * [亮度Y(4) ＋ U(1) + V(1)]/4(像素) = 3/2bit
     * 所有I420单帧图片要占用的内存为：int frameSize = HEIGHT * WIDTH * 3 / 2
     * 接着就是分配一个无符号的内存空间，来存储从文件中读取的yuv数据。
     * */
    FILE *pFile = fopen("/Users/linchengzhong/Desktop/tiger_yuv420p_1000x667.yuv", "rb");
    if (pFile == NULL) {
        cerr << "tiger_yuv420p_1000x667.yuv open failed" << endl;
    }
    unsigned char *m_yuv_data;
    int frameSize = HEIGHT * WIDTH * 3 / 2; // 单帧数据的bit数
    m_yuv_data = (unsigned char *) malloc(frameSize * sizeof(unsigned char));
    fread(m_yuv_data, 1, frameSize, pFile);
    fclose(pFile);

    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // 创建纹理
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (texture != NULL) {
        SDL_Event windowEvent;
        while (true) {
            if (SDL_PollEvent(&windowEvent)) {
                if (SDL_QUIT == windowEvent.type) {
                    cout << "SDL quit!!" << endl;
                    break;
                }
            }
            SDL_UpdateTexture(texture, NULL, m_yuv_data, WIDTH); // 更新纹理
            SDL_RenderClear(renderer); // 清除渲染器
            SDL_RenderCopy(renderer, texture, NULL, NULL); // 拷贝渲染器到纹理。
            SDL_RenderPresent(renderer); // 渲染
        }

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
}
