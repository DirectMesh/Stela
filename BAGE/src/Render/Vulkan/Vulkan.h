#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <optional>

class Vulkan {
    public:

    VkInstance Instance;
    const VkAllocationCallbacks* pAllocator = nullptr;
    VkDebugUtilsMessengerEXT DebugMessenger;
    VkDevice Device;
    VkPhysicalDeviceFeatures DeviceFeatures{};
    VkQueue GraphicsQueue;
    VkSurfaceKHR Surface;
    VkQueue PresentQueue;
    

    #ifdef NDEBUG
        const bool EnableValidationLayers = false;
    #else
        const bool EnableValidationLayers = true;
    #endif

    const std::vector<const char*> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamly;

        bool IsComplete() {
            return graphicsFamily.has_value() && presentFamly.has_value();
        }
    };

    void Init(SDL_Window *window);
    void CreateInstance();
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    void SetupDebugMessenger();
    void CreateSurface(SDL_Window *window);
    void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* pAllocator);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);
    void PickPhysicalDevice();
    int RateDeviceSuitability(VkPhysicalDevice Device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    void CreateLogicalDevice();
    void Cleanup();
};