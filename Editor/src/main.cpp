#include <Stela.h>
#include <DynamicLibrary.h>
#include <Scripts/ScriptsAPI.h>
#include <Scripts/RegisterSystem.h>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <filesystem>
#include <chrono>

#include <SDL3/SDL.h>

#if !defined(__APPLE__)
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#else
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_metal.h"
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// Platform helpers

static fs::path GetExeDir()
{
#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    _NSGetExecutablePath(path.data(), &size);
    return fs::path(path).parent_path();
#elif defined(__linux__)
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
    return fs::path(std::string(buffer, len)).parent_path();
#endif
}

#if defined(_WIN32)
static constexpr const char* ext = ".dll";
#elif defined(__APPLE__)
static constexpr const char* ext = ".dylib";
#else
static constexpr const char* ext = ".so";
#endif

// Globals

std::atomic<bool> quit{false};
extern std::atomic<bool> enginePaused;

static ScriptsAPI gEngineAPI{};

void* scriptModule = nullptr;
void (*initFn)(ScriptsAPI*) = nullptr;
void (*shutdownFn)() = nullptr;

fs::path exeDir;
fs::path scriptSourceFolder;
fs::path scriptBuildLibPath;
fs::path scriptLiveLibPath;
fs::path cmakeBuildDir;

fs::file_time_type lastScriptWriteTime;

// SDL event watch used so Editor can process events before the engine
static bool SDLEventWatch(void* userdata, SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
    return true;
}

// Logging

void EngineLog(const char* msg)
{
    std::cout << "[Stela] " << msg << std::endl;
}

// Validation

static bool ValidateRegisteredSystems()
{
#if defined(__APPLE__) || defined(__linux__)
    Dl_info info{};
    for (auto& sys : gScriptSystems)
    {
        void* fn = (void*)sys.Update ? (void*)sys.Update : sys.Start ? (void*)sys.Start : (void*)sys.Shutdown;
        if (!fn)
            return false;

        if (!dladdr(fn, &info) || !info.dli_fname)
            return false;

        std::string expected = std::string("Scripts") + ext;
        if (std::string(info.dli_fname).find(expected) == std::string::npos)
            return false;
    }
#endif
    return true;
}

// Script change detection

bool ScriptsChanged()
{
    try
    {
        fs::file_time_type latest = fs::file_time_type::min();

        for (auto& p : fs::recursive_directory_iterator(scriptSourceFolder))
        {
            if (p.is_regular_file())
                latest = std::max(latest, fs::last_write_time(p));
        }

        if (latest > lastScriptWriteTime)
        {
            lastScriptWriteTime = latest;
            return true;
        }
    }
    catch (...)
    {
    }

    return false;
}

// Reload logic

bool ReloadScripts(Stela* engine)
{
    if (scriptModule)
    {
        RunShutdowns();
        shutdownFn();
        DynamicLibrary::Unload(scriptModule);
        scriptModule = nullptr;
    }

    std::cout << "[Editor] Building Scripts...\n";

    std::string buildCmd =
        "cmake --build \"" + cmakeBuildDir.string() + "\" --target Scripts";

    if (system(buildCmd.c_str()) != 0)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Scripts Reload Failed",
            "Building Scripts failed.\nCheck console output.",
            engine->Window
        );
        return false;
    }

#if defined(_WIN32)
    fs::copy_file(
        scriptBuildLibPath,
        scriptLiveLibPath,
        fs::copy_options::overwrite_existing
    );
    scriptModule = DynamicLibrary::Load(scriptLiveLibPath.string().c_str());
#else
    scriptModule = DynamicLibrary::Load(scriptBuildLibPath.string().c_str());
#endif

    if (!scriptModule)
        return false;

    initFn = (void (*)(ScriptsAPI*))DynamicLibrary::GetSymbol(scriptModule, "Scripts_Init");
    shutdownFn = (void (*)())DynamicLibrary::GetSymbol(scriptModule, "Scripts_Shutdown");

    if (!initFn || !shutdownFn)
        return false;

    gEngineAPI.Version = 1;
    gEngineAPI.Log = EngineLog;
    gEngineAPI.RegisterScript = Engine_RegisterScript;

    initFn(&gEngineAPI);
    RunStarts();
    return ValidateRegisteredSystems();
}

// Main

int main()
{
    Stela engine;
    engine.Init("Stela Editor", 1920, 1080);

    // ImGui Initialization
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#if !defined(__APPLE__)
    // ImGui Vulkan integration (Editor-only)
    // Initialize SDL3 platform backend
    ImGui_ImplSDL3_InitForVulkan(engine.Window);

    // Create descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiDescriptorPool;
    if (vkCreateDescriptorPool(engine.vulkan.Device, &pool_info, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
    {
        std::cerr << "Failed to create ImGui descriptor pool" << std::endl;
        return 1;
    }

    // Setup ImGui Vulkan init info (new backend API expects all pipeline info inside PipelineInfoMain)
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = engine.vulkan.Instance;
    init_info.PhysicalDevice = engine.vulkan.PhysicalDevice;
    init_info.Device = engine.vulkan.Device;
    // find graphics queue family index
    auto qf = engine.vulkan.FindQueueFamilies(engine.vulkan.PhysicalDevice);
    init_info.QueueFamily = qf.graphicsFamily.value();
    init_info.Queue = engine.vulkan.GraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.DescriptorPoolSize = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = static_cast<uint32_t>(engine.vulkan.swapChainImages.size());
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;

    // Set the RenderPass in the PipelineInfoMain structure (newer API)
    init_info.PipelineInfoMain.RenderPass = engine.vulkan.RenderPass;

    ImGui_ImplVulkan_Init(&init_info);

    // NOTE: Modern imgui Vulkan backend will create and upload font texture internally
    // on first NewFrame() call. No explicit font-upload command buffer is required here.

    // Event watch so Editor can receive SDL events before engine polls them
    SDL_AddEventWatch(SDLEventWatch, nullptr);

    // Set Vulkan command recording callback to render ImGui within engine render pass
    engine.vulkan.ImGuiRenderCallback = [&](VkCommandBuffer cmd)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    };
#else
    // ImGui Metal integration
    ImGui_ImplSDL3_InitForMetal(engine.Window);
    ImGui_ImplMetal_Init(engine.metal.Device);
    
    // Set Metal callback
    engine.metal.ImGuiRenderCallback = [&](MTL::RenderCommandEncoder* encoder) {
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), engine.metal.metalCommandBuffer, encoder);
    };
    
    // Event watch for Metal as well
    SDL_AddEventWatch(SDLEventWatch, nullptr);
#endif

    exeDir = GetExeDir();
    std::string libName = std::string("Scripts") + ext;

    std::vector<fs::path> searchPaths;
    fs::path currentPath = exeDir;
    // Search up to 6 levels up to handle MacOS app bundle structure
    // .../Stela/bin/Stela_EDITOR.app/Contents/MacOS -> .../Stela
    for (int i = 0; i < 6; ++i) {
        searchPaths.push_back(currentPath);
        if (currentPath.has_parent_path())
            currentPath = currentPath.parent_path();
        else
            break;
    }

    // Scripts source folder
    for (auto& base : searchPaths)
    {
        auto p = base / "Scripts";
        if (fs::exists(p) && fs::is_directory(p))
            scriptSourceFolder = p;
    }

    // Compiled library
    for (auto& base : searchPaths)
    {
        for (auto& sub : { base / libName, base / "bin" / libName })
        {
             if (fs::exists(sub) && !fs::is_directory(sub))
                scriptBuildLibPath = sub;
        }
    }

    // CMake build directory
    for (auto& base : searchPaths)
    {
        auto p = base / "build";
        if (fs::exists(p) && fs::is_directory(p))
            cmakeBuildDir = p;
    }

#if defined(_WIN32)
    scriptLiveLibPath = scriptBuildLibPath.parent_path() / "Scripts_live.dll";
#else
    scriptLiveLibPath = scriptBuildLibPath;
#endif

    if (scriptSourceFolder.empty() ||
        scriptBuildLibPath.empty() ||
        cmakeBuildDir.empty())
    {
        std::cerr << "[Editor] Failed to resolve paths\n";
        return 1;
    }

#if defined(_WIN32)
    fs::copy_file(
        scriptBuildLibPath,
        scriptLiveLibPath,
        fs::copy_options::overwrite_existing
    );
    scriptModule = DynamicLibrary::Load(scriptLiveLibPath.string().c_str());
#else
    scriptModule = DynamicLibrary::Load(scriptBuildLibPath.string().c_str());
#endif

    initFn = (void (*)(ScriptsAPI*))DynamicLibrary::GetSymbol(scriptModule, "Scripts_Init");
    shutdownFn = (void (*)())DynamicLibrary::GetSymbol(scriptModule, "Scripts_Shutdown");

    gEngineAPI.Version = 1;
    gEngineAPI.Log = EngineLog;
    gEngineAPI.RegisterScript = Engine_RegisterScript;

    initFn(&gEngineAPI);
    RunStarts();
    ValidateRegisteredSystems();

    lastScriptWriteTime = fs::file_time_type::min();
    ScriptsChanged();

    std::thread watcher([&]()
    {
        while (!quit)
        {
            if (ScriptsChanged())
            {
                engine.bPauseRun = true;
                while (!enginePaused)
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                if (ReloadScripts(&engine))
                    engine.bPauseRun = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

        while (!quit)
    {
        // Begin ImGui frame (Editor-only)
        #if !defined(__APPLE__)
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        #else
        // For Metal, we need a render pass descriptor for the new frame to deduce pixel format.
        // We create a dummy texture and descriptor.
        MTL::TextureDescriptor* textureDesc = MTL::TextureDescriptor::alloc()->init();
        textureDesc->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        textureDesc->setWidth(1);
        textureDesc->setHeight(1);
        MTL::Texture* dummyTexture = engine.metal.Device->newTexture(textureDesc);
        textureDesc->release();

        MTL::RenderPassDescriptor* imguiPassDesc = MTL::RenderPassDescriptor::alloc()->init();
        imguiPassDesc->colorAttachments()->object(0)->setTexture(dummyTexture);
        
        ImGui_ImplMetal_NewFrame(imguiPassDesc);
        
        imguiPassDesc->release();
        dummyTexture->release();
        
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        #endif

        // Full-window dockspace host
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        // Make host window fully transparent so the central dock node does not dim the renderer
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpace Host", nullptr, host_flags);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        // Example Editor window docked into the dockspace
        ImGui::Begin("Editor");
        ImGui::Text("Stela Editor - ImGui overlay");
        ImGui::Text("FPS: %.1f", 1.0f / engine.deltaTime);
        ImGui::End();

        ImGui::End(); // DockSpace Host

        ImGui::Render();

        engine.RunFrame();
        if (engine.bQuit)
            quit = true;
    }

    watcher.join();

#if !defined(__APPLE__)
    // Ensure GPU is idle, then shutdown ImGui and destroy descriptor pool
    if (engine.vulkan.Device)
        vkDeviceWaitIdle(engine.vulkan.Device);

    // Remove render callback before shutdown
    engine.vulkan.ImGuiRenderCallback = {};

    // Remove the SDL event watch so no events are forwarded after backend shutdown
    SDL_RemoveEventWatch(SDLEventWatch, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // Destroy descriptor pool created for ImGui
    if (imguiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(engine.vulkan.Device, imguiDescriptorPool, nullptr);
        imguiDescriptorPool = VK_NULL_HANDLE;
    }
#else
    // Remove the SDL event watch so no events are forwarded after backend shutdown
    SDL_RemoveEventWatch(SDLEventWatch, nullptr);

    engine.metal.ImGuiRenderCallback = {};
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
#endif

    engine.Cleanup();

    RunShutdowns();
    shutdownFn();
    DynamicLibrary::Unload(scriptModule);
    return 0;
}
