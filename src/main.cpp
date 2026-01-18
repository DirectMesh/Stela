#include "BAGE.h"
#include <SDL3/SDL.h>
#ifndef APPLE
#include <SDL3/SDL_vulkan.h>
#else
#include <SDL3/SDL_metal.h>
#endif
#include <thread>

void BAGE::Init()
{
    SDL_Init(SDL_INIT_VIDEO);

    #ifndef APPLE
    SDL_WindowFlags WindowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    #else
    SDL_WindowFlags WindowFlags = (SDL_WindowFlags)(SDL_WINDOW_METAL);
    #endif

    SDL_Window *Window = SDL_CreateWindow("BAGE", 1920, 1080, WindowFlags);

    SDL_Event e;
    bool bQuit = false;
    bool stop_rendering = false;

    // main loop
    while (!bQuit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_EVENT_QUIT)
                bQuit = true;

            if (e.type == SDL_WINDOW_MINIMIZED) {
                    stop_rendering = true;
                }
                if (e.type != SDL_WINDOW_MINIMIZED) {
                    stop_rendering = false;
                }
            }
        }

        // do not draw if we are minimized
        if (stop_rendering) {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
}