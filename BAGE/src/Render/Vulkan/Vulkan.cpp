#if !defined(__APPLE__)

#include "Vulkan.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <SDL3/SDL.h>

// File-scoped physical device used by PickPhysicalDevice and CreateLogicalDevice
static VkPhysicalDevice gPhysicalDevice = VK_NULL_HANDLE;

void Vulkan::Init(SDL_Window *window)
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface(window);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain(window);
}

void Vulkan::CreateInstance()
{
    if (EnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "BAGE";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "BAGE";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t apiVersion = VK_API_VERSION_1_0;

    auto enumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(
            nullptr, "vkEnumerateInstanceVersion");

    if (enumerateInstanceVersion)
    {
        enumerateInstanceVersion(&apiVersion);
    }

    AppInfo.apiVersion = apiVersion;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
    if (EnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();

        PopulateDebugMessengerCreateInfo(DebugCreateInfo);
        CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&DebugCreateInfo;
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;

        CreateInfo.pNext = nullptr;
    }

    auto extensions = GetRequiredExtensions();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    CreateInfo.ppEnabledExtensionNames = extensions.data();

    for (const auto &extension : extensions)
    {
        std::cout << '\t' << extension << '\n';
    }

    VkResult Result = vkCreateInstance(
        &CreateInfo,
        pAllocator,
        &Instance);

    if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

bool Vulkan::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : ValidationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char *> Vulkan::GetRequiredExtensions()
{
    uint32_t sdlExtensionCount = 0;
    const char *const *sdlExtensions =
        SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    if (!sdlExtensions)
    {
        throw std::runtime_error("Failed to get Vulkan extensions from SDL");
    }

    std::vector<const char *> extensions;
    extensions.reserve(sdlExtensionCount + 1);

    for (uint32_t i = 0; i < sdlExtensionCount; ++i)
    {
        extensions.push_back(sdlExtensions[i]);
    }

    if (EnableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::DebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(Instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Vulkan::SetupDebugMessenger()
{
    if (!EnableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Vulkan::DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(Instance, DebugMessenger, pAllocator);
    }
}

void Vulkan::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &CreateInfo)
{
    CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = DebugCallBack;
    CreateInfo.pUserData = nullptr;
}

void Vulkan::CreateSurface(SDL_Window* window)
{
    if (!SDL_Vulkan_CreateSurface(window, Instance, pAllocator, &Surface))
    {
        throw std::runtime_error(
            std::string("Failed to create Vulkan surface: ") + SDL_GetError()
        );
    }
}

void Vulkan::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    std::cout << "Detected Vulkan devices:\n";

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = RateDeviceSuitability(device);
        candidates.insert({ score, device });

        std::cout << "  - " << props.deviceName
                  << " (type=" << props.deviceType
                  << ", score=" << score << ")\n";
    }

    // Pick the best scoring device
    auto bestCandidate = candidates.rbegin();
    if (bestCandidate->first < 0) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    gPhysicalDevice = bestCandidate->second;

    VkPhysicalDeviceProperties chosenProps;
    vkGetPhysicalDeviceProperties(gPhysicalDevice, &chosenProps);

    std::cout << "Selected GPU: " << chosenProps.deviceName << std::endl;
}

int Vulkan::RateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &feats);

    if (!IsDeviceSuitable(device)) {
        return -1;
    }

    int score = 0;

    switch (props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 10'000;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 5'000;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            // Strongly discourage llvmpipe
            score -= 10'000;
            break;

        default:
            break;
    }

    // Minor influence from limits
    score += static_cast<int>(props.limits.maxImageDimension2D / 1024);

    return score;
}

Vulkan::QueueFamilyIndices Vulkan::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        const auto& queueFamily = queueFamilies[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Surface, &presentSupport);

        if (presentSupport)
            indices.presentFamly = i;

        if (indices.IsComplete())
            break;
    }

    return indices;
}

bool Vulkan::IsDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool ExtensionSupported = CheckExtensionSupport(device);

    bool swapChainAdequate = false;
    
    if (ExtensionSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && ExtensionSupported && swapChainAdequate;
}

bool Vulkan::CheckExtensionSupport(VkPhysicalDevice Device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Vulkan::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(gPhysicalDevice);

    // Create a set of unique queue families we need (graphics + present)
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamly.value() };

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &DeviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        createInfo.ppEnabledLayerNames = ValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(gPhysicalDevice, &createInfo, nullptr, &Device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    // Retrieve queues from the created logical device
    vkGetDeviceQueue(Device, indices.graphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(Device, indices.presentFamly.value(), 0, &PresentQueue);
}

Vulkan::SwapChainSupportDetails Vulkan::querySwapChainSupport(VkPhysicalDevice Device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Vulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvalableFormats) {
    for (const auto& AvalableFormats : AvalableFormats) {
            if (AvalableFormats.format == VK_FORMAT_B8G8R8A8_SRGB && AvalableFormats.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return AvalableFormats;
            }
        }

        return AvalableFormats[0];
}

VkPresentModeKHR Vulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes) {
        for (const auto& availablePresentMode : AvailablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, SDL_Window* Window) {
    if (Capabilities.currentExtent.width != UINT32_MAX) {
        return Capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSizeInPixels(Window, &width, &height); // Gets framebuffer size in pixels

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = SDL_clamp(actualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        actualExtent.height = SDL_clamp(actualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Vulkan::CreateSwapChain(SDL_Window* Window) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(gPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, Window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(gPhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamly.value()};

    if (indices.graphicsFamily != indices.presentFamly) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(Device, &createInfo, nullptr, &SwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, swapChainImages.data());

    SwapChainImageFormat = surfaceFormat.format;
    SwapChainExtent = extent;
}

void Vulkan::Cleanup()
{
    vkDestroySwapchainKHR(Device, SwapChain, nullptr);
    vkDestroyDevice(Device, nullptr);
    if (EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, pAllocator);
    }

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

#endif