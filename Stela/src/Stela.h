#pragma once
#include <SDL3/SDL.h>

#if defined(__APPLE__)
    #include "Render/Metal/Metal.h"
#else
    #include "Render/Vulkan/Vulkan.h"
#endif

class Stela {
    public:

    SDL_Window *Window = nullptr;
    
    #if defined(__APPLE__)
        Metal metal;
    #else
        Vulkan vulkan;
    #endif
    
    void Init(const char* appName = "Stela", int width = 1920, int height = 1080);
    void Run();
    void Cleanup();
};