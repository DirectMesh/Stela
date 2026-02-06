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
    VkPipeline SwapChainPipeline;
    std::vector<VkFramebuffer> SwapChainFramebuffers;
    VkCommandPool CommandPool;
    std::vector<VkCommandBuffer> CommandBuffers;
    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkFence> InFlightFences;
    uint32_t currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    // Offscreen Resources
    VkImage OffscreenImage;
    VkDeviceMemory OffscreenImageMemory;
    VkImageView OffscreenImageView;
    VkSampler OffscreenSampler;
    VkFramebuffer OffscreenFramebuffer;
    VkRenderPass OffscreenRenderPass;
    VkDescriptorSet OffscreenDescriptorSet = VK_NULL_HANDLE;

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
    
    void RecordSceneCommands(VkCommandBuffer commandBuffer, VkPipeline pipeline);
    bool CheckExtensionSupport(VkPhysicalDevice Device);
    void CreateLogicalDevice();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice Device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &AvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &AvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &Capabilities, SDL_Window *Window);
    void CreateSwapChain(SDL_Window *Window);
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule CreateShaderModule(const std::vector<char> &code);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void CreateImageViews();
    void CreateRenderPass();
    void CreateOffscreenResources(); // New
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