#include "Stela.h"
#include <SDL3/SDL.h>
#if defined(__APPLE__) && (defined(TARGET_OS_IPHONE) || defined(TARGET_OS_TV))
#include <SDL3/SDL_metal.h> // For iOS/tvOS Metal rendering
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
#include "Scripts/ScriptsAPI.h"
#include "Scripts/RegisterSystem.h"
#include <atomic>

std::atomic<bool> enginePaused{false};
#include <thread>
#include <iostream>
#include <iomanip>

void Stela::Init(const char *appName, int width, int height)
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

    lastTime = SDL_GetPerformanceCounter();
}

void Stela::Run()
{
    while (!bQuit)
    {
        RunFrame();
    }
}

void Stela::RunFrame()
{
    if (bQuit)
        return; // engine requested to quit

    SDL_Event e;
    bool stop_rendering = false;

    // Handle all SDL events for this frame
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_EVENT_QUIT)
        {
            bQuit = true; // can be checked by Editor
        }

        if (e.type == SDL_WINDOW_MINIMIZED)
        {
            stop_rendering = true;
        }
        else
        {
            stop_rendering = false;
        }
    }

    // Pause if Editor requested pause (e.g., hot-reload)
    if (bPauseRun)
    {
        extern std::atomic<bool> enginePaused;
        enginePaused = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return;
    }
    {
        extern std::atomic<bool> enginePaused;
        enginePaused = false;
    }

    // Skip frame if window minimized
    if (stop_rendering)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    // DeltaTime
    uint64_t now = SDL_GetPerformanceCounter();
    deltaTime = (now - lastTime) / (float)SDL_GetPerformanceFrequency();
    lastTime = now;

    // Clamp deltaTime
    if (deltaTime < 0.001f)
        deltaTime = 0.001f;
    if (deltaTime > 0.05f)
        deltaTime = 0.05f;

    // Run all engine systems
    RunSystems(deltaTime);

#if defined(__APPLE__)
    // Add Metal DrawFrame
#else
    vulkan.DrawFrame();
#endif
}

void Stela::Cleanup()
{
// First, clean up renderer
#if defined(__APPLE__)
    metal.Cleanup(); // use the instance, not the class
#else
    vkDeviceWaitIdle(vulkan.Device);
    vulkan.Cleanup();
#endif

    // Then destroy SDL window and quit
    if (Window)
    {
        SDL_DestroyWindow(Window);
        Window = nullptr;
    }
    SDL_Quit();
}
