#include <iostream>
#include <vector>
//This is written using raylib, but we can use standard c++
#include <raylib.h>

void null(int, const char*, va_list);
void calc(Image *i, int *dx, int *dy);

using namespace std;

int main() {
    SetTraceLogCallback(null);

    if (!FileExists("./out.vpr")) {
        cout << "The video file (./out.vpr) doesn't exist, please generate one first" << "\n";
        return 1;
    }

    uint32_t dataSize;
    //Load the file
    unsigned char *f = LoadFileData("./out.vpr", &dataSize);
    //Copy the pointer "f" (we are going to increment it, so we can move through the file) to "source"
    unsigned char *source = f;
    uint32_t oSize = 0;
    uint32_t fps = 0;

    //The first 4 bytes contain the FPS value of the video
    ((unsigned char*) &fps)[0] = f[0];
    ((unsigned char*) &fps)[1] = f[1];
    ((unsigned char*) &fps)[2] = f[2];
    ((unsigned char*) &fps)[3] = f[3];
    f+=(char)4;
    //After the FPS the other 4 bytes contain the amount of frames
    ((unsigned char*) &oSize)[0] = f[0];
    ((unsigned char*) &oSize)[1] = f[1];
    ((unsigned char*) &oSize)[2] = f[2];
    ((unsigned char*) &oSize)[3] = f[3];
    f+=(char)4;

    vector<Image> buffer;
    buffer.reserve(oSize);

    cout << "Reading frames please wait..." << "\n";
    for (unsigned int i = 0; i < oSize; i++) {
        uint16_t frameNum = 0;
        //4 bytes contain the N of this frame (from 0 to oSize) so you can check whether everything is working fine
        ((unsigned char*) &frameNum)[0] = f[0];
        ((unsigned char*) &frameNum)[1] = f[1];
        f+=2;
        uint32_t frameSize = 0;
        //The next 4 bytes contain the size of the "encoded" frame
        ((unsigned char*) &frameSize)[0] = f[0];
        ((unsigned char*) &frameSize)[1] = f[1];
        ((unsigned char*) &frameSize)[2] = f[2];
        ((unsigned char*) &frameSize)[3] = f[3];
        f+=4;
        cout << "MEM: " << (void*)f << " | Size: " << frameSize << " | Number: " << frameNum << "\n";
        //Load the frame from the data
        buffer.push_back(LoadImageFromMemory(".png", f, (int) frameSize));
        //Skip to the next frame
        f+=frameSize;
    }

    // ----- Do stuff with the frames ----- //

    //Example VIDEO PLAYER written using RayLib (openGL)
    char c;
    cout << "Would you like to view the video? Y=yes >> ";
    cin >> c;
    if (c == 'Y' || c == 'y') {
        InitAudioDevice();
        while (!IsAudioDeviceReady());
        SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(buffer[0].width, buffer[1].height, "");
        SetTargetFPS((int) fps);
        unsigned int current = 1;
        int dx, dy;
        while (current != oSize && !WindowShouldClose()) {
            calc(&buffer[current - 1], &dx, &dy);
            BeginDrawing();
            ClearBackground(BLACK);
            Texture t = LoadTextureFromImage(buffer[current - 1]);

            DrawTexturePro(t, {0, 0, (float) t.width, (float) t.height}, {0, 0, (float) dx, (float) dy}, {0, 0}, 0,
                           WHITE);

            EndDrawing();
            UnloadTexture(t);
            current++;
        }
        CloseWindow();
        CloseAudioDevice();
    }

    //Remember to clear up the memory
    UnloadFileData(source); //Remember to clear "source" and not "f" (as F points to the end of the file now) for safety
    for (const auto &item: buffer) UnloadImage(item);
    buffer.clear();
    buffer.shrink_to_fit();

    //Return
    return 0;
}

void null(int, const char*, va_list) {}

//Try fitting the frame in the window
void calc(Image *i, int *dx, int *dy) {
    *dx = GetScreenWidth();
    *dy = GetScreenHeight();
    if (*dx < *dy) *dy = *dx / i->width * i->height; else *dx = *dy / i->height * i->width;
}
