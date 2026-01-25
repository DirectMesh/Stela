#include "Stela.h"
#include <SDL3/SDL.h>
#if defined(__APPLE__) && (defined(TARGET_OS_IPHONE) || defined(TARGET_OS_TV))
    #include <SDL3/SDL_metal.h>  // For iOS/tvOS Metal rendering
#elif defined(__ANDROID__)
    #include <SDL3/SDL_vulkan.h> // Vulkan on Android
#else
    // Other platforms (Windows, Linux, macOS)
    #if defined(__APPLE__)
        #include <SDL3/SDL_metal.h>
    #else
        #include <SDL3/SDL_vulkan.h>
    #endif
#endif
#include <thread>
#include <iostream>

void Stela::Init(const char* appName, int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    #if defined(__APPLE__)
        std::cout << "Using Metal Renderer" << std::endl;
        SDL_WindowFlags WindowFlags = (SDL_WindowFlags)(SDL_WINDOW_METAL);
    #else
        std::cout << "Using Vulkan Renderer" << std::endl;
        SDL_WindowFlags WindowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    #endif

    Window = SDL_CreateWindow(appName, width, height, WindowFlags);

    #if defined(__APPLE__)
        metal.Init();
    #else
        vulkan.Init(Window);
    #endif
}

void Stela::Run() {
    SDL_Event e;
    bool bQuit = false;
    bool stop_rendering = false;

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

void Stela::Cleanup()
{
    // First, clean up renderer
    #if defined(__APPLE__)
        metal.Cleanup();      // use the instance, not the class
    #else
        vulkan.Cleanup();
    #endif

    // Then destroy SDL window and quit
    if (Window) {
        SDL_DestroyWindow(Window);
        Window = nullptr;
    }
    SDL_Quit();
}
