#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;
const int WIDTH = 1000, HEIGHT = 667;

int main(int argc, char *argv[]) {
    SDL_Surface *imageSurface = NULL;
    SDL_Surface *windowSurface = NULL;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("Hello SDL world!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH,
                                          HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (NULL == window) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        cout << "SDL_image could not init with error: " << IMG_GetError() << endl;
        return -1;
    }
    windowSurface = SDL_GetWindowSurface(window);
    imageSurface = IMG_Load(argv[1]);
    if (NULL == imageSurface) {
        cout << "SDL could not load image with error: " << SDL_GetError() << endl;
        return -1;
    }
    SDL_Event windowEvent;
    int count = 0;
    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (SDL_QUIT == windowEvent.type) {
                cout << "SDL quit!!" << endl;
                break;
            }
        }
        count++;
        cout << "while count" << count << endl;
        SDL_BlitSurface(imageSurface, NULL, windowSurface, NULL);
        SDL_UpdateWindowSurface(window);
    }

    imageSurface = NULL;
    windowSurface = NULL;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
