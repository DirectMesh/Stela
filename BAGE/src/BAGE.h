#pragma once
#include <SDL3/SDL.h>

#ifndef APPLE
    #include "Render/Vulkan/Vulkan.h"
#else

#endif

class BAGE {
    public:

    SDL_Window *Window = nullptr;
    
    void Init(const char* appName = "BAGE", int width = 1920, int height = 1080);
    void Run();
    void Cleanup();

    enum class RenderType {
        Vulkan,
        Metal
    };

    RenderType RendererType;
};