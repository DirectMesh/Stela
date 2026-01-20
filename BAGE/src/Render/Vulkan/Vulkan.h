#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

class Vulkan {
    public:

    VkInstance Instance;

    void Init();
    void CreateInstance();
    void Cleanup();
};