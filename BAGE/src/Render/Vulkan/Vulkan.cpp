#include "Vulkan.h"
#include <stdexcept>
#include <vector>
#include <iostream>

void Vulkan::Init() {
    CreateInstance();
}

void Vulkan::CreateInstance() {
    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "BAGE";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "BAGE";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    uint32_t ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, extensions.data());
    const char* const* Extensions = SDL_Vulkan_GetInstanceExtensions(&ExtensionCount);
    if (Extensions == NULL) {
        throw std::runtime_error("Failed to get Vulkan extensions from SDL");
    }

    CreateInfo.enabledExtensionCount = ExtensionCount;
    CreateInfo.ppEnabledExtensionNames = Extensions;

    CreateInfo.enabledLayerCount = 0;

    std::cout << "available extensions:\n";

    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }

    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &Instance);

    if (Result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void Vulkan::Cleanup() {
    vkDestroyInstance(Instance, nullptr);
}