#pragma once
#include <SDL3/SDL.h>

#ifdef APPLE
    #include "Render/Vulkan/Vulkan.h"
#else
    #include "Render/Metal/Metal.h"
#endif

class BAGE {
    public:

    SDL_Window *Window = nullptr;
    
    #ifdef APPLE
        Vulkan vulkan;
    #else
        Metal metal;
    #endif
    
    void Init(const char* appName = "BAGE", int width = 1920, int height = 1080);
    void Run();
    void Cleanup();
};
