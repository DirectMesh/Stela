#pragma once

#if !defined(__APPLE__)

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <optional>
#include <fstream>
#include <functional>

class Vulkan
{
public:
    // Optional callback used by external code (Editor) to record additional render commands
    std::function<void(VkCommandBuffer)> ImGuiRenderCallback;
    VkInstance Instance;
    const VkAllocationCallbacks *pAllocator = nullptr;
    VkDebugUtilsMessengerEXT DebugMessenger;
    VkDevice Device;
    VkPhysicalDeviceFeatures DeviceFeatures{};
    VkQueue GraphicsQueue;
    VkSurfaceKHR Surface;
    VkQueue PresentQueue;
    VkSwapchainKHR SwapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat SwapChainImageFormat;
    VkExtent2D SwapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass RenderPass;
    VkPipelineLayout PipelineLayout;
    VkPipeline GraphicsPipeline;
    std::vector<VkFramebuffer> SwapChainFramebuffers;
    VkCommandPool CommandPool;
    VkCommandBuffer CommandBuffer;
    VkSemaphore ImageAvailableSemaphore;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    VkFence InFlightFence;

#ifdef NDEBUG
    const bool EnableValidationLayers = false;
#else
    const bool EnableValidationLayers = true;
#endif

    const std::vector<const char *> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamly;

        bool IsComplete()
        {
            return graphicsFamily.has_value() && presentFamly.has_value();
        }
    };

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    void Init(SDL_Window *window);
    void CreateInstance();
    bool CheckValidationLayerSupport();
    std::vector<const char *> GetRequiredExtensions();
    void SetupDebugMessenger();
    void CreateSurface(SDL_Window *window);
    void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks *pAllocator);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &CreateInfo);
    void PickPhysicalDevice();
    int RateDeviceSuitability(VkPhysicalDevice Device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool CheckExtensionSupport(VkPhysicalDevice Device);
    void CreateLogicalDevice();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice Device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &AvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &AvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &Capabilities, SDL_Window *Window);
    void CreateSwapChain(SDL_Window *Window);
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule CreateShaderModule(const std::vector<char> &code);
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void CreateSyncObjects();
    void DrawFrame();
    void Cleanup();

    // Expose selected physical device for external use (e.g. Editor ImGui init)
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
};

#endif