#include <iostream>

#include <SDL.h>

using namespace std;

int main() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }

    SDL_AudioSpec desired_spec;
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 2;
    desired_spec.silence = 0;
    desired_spec.samples = 1024;
    desired_spec.callback = NULL;

    SDL_AudioDeviceID deviceID;
    if ((deviceID = SDL_OpenAudioDevice(NULL, 0, &desired_spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2) {
        cout << "SDL_OpenAudioDevice with error deviceID : " << deviceID << endl;
        return -1;
    }

    FILE *pFile = fopen("/Users/linchengzhong/Desktop/MyHeartWillGoOn.pcm", "rb");
    if (pFile == NULL) {
        cerr << "ForeverMore_44100_2_16.pcm open failed" << endl;
    }
    Uint32 buffer_size = 4096;
    char *buffer = (char *) malloc(buffer_size);
    SDL_PauseAudioDevice(deviceID, 0);
    while (true) {
        if (fread(buffer, 1, buffer_size, pFile) != buffer_size) {
            cerr << "end of file" << endl;
            break;
        }
        SDL_QueueAudio(deviceID, buffer, buffer_size);
    }
    SDL_Delay(5 * 1000 * 60); // 暂停5分钟，等待播放完成。在做播放器时，可以通过ffmpeg获取duration时间，做适当延迟。
    SDL_CloseAudio();
    SDL_Quit();
    fclose(pFile);
}
