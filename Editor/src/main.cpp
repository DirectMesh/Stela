#include <Stela.h>
#include <Scripts/ScriptEngine.h>
#include <Scripts/RegisterSystem.h>
#include <Scripts/EngineGlobals.h>

#include <iostream>
#include <string>
#include <vector>
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

#include "imgui_internal.h"

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
    std::vector<char> buffer(size);
    _NSGetExecutablePath(buffer.data(), &size);
    return fs::path(buffer.data()).parent_path();
#elif defined(__linux__)
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
    return fs::path(std::string(buffer, len)).parent_path();
#endif
}

// Globals

std::atomic<bool> quit{false};
std::atomic<bool> enginePaused{false};

fs::path exeDir;
fs::path scriptFolder;

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

// Script change detection

bool ScriptsChanged()
{
    try
    {
        fs::file_time_type latest = fs::file_time_type::min();

        // Check for changes in Scripts folder
        if (fs::exists(scriptFolder)) {
            for (auto it = fs::recursive_directory_iterator(scriptFolder); it != fs::recursive_directory_iterator(); ++it)
            {
                if (it->is_directory()) {
                    std::string name = it->path().filename().string();
                    if (name == "bin" || name == "obj" || name == ".git" || name == ".vs") {
                        it.disable_recursion_pending();
                    }
                }
                else if (it->is_regular_file()) {
                    auto ext = it->path().extension();
                    if (ext == ".cs" || ext == ".csproj")
                        latest = std::max(latest, fs::last_write_time(*it));
                }
            }
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
    std::cout << "[Editor] Reloading Scripts...\n";

    // 1. Shutdown existing scripts
    RunShutdowns();
    ScriptEngine::Shutdown();
    gScriptSystems.clear();

    // 2. Build Scripts (dotnet build)
    // We cd into the scriptFolder and run dotnet build
    // scriptFolder is exeDir / "Scripts"
    std::string buildCmd = "cd \"" + scriptFolder.string() + "\" && dotnet build -c Debug";
    
    std::cout << "[Editor] Running: " << buildCmd << std::endl;
    if (system(buildCmd.c_str()) != 0)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Scripts Build Failed",
            "dotnet build failed.\nCheck console output.",
            engine->Window
        );
        return false;
    }

    // 3. Copy artifacts to Exe dir
    // We assume the build output is in scriptFolder/bin/Debug/net10.0/ or similar
    // We look for UserScripts.dll and UserScripts.runtimeconfig.json recursively in scriptFolder/bin
    fs::path dllPath;
    fs::path configPath;
    
    // Safety check: bin folder must exist
    if (!fs::exists(scriptFolder / "bin")) {
        std::cerr << "[Editor] Build succeeded but 'bin' folder missing in " << scriptFolder << std::endl;
        return false;
    }

    for(auto& p : fs::recursive_directory_iterator(scriptFolder / "bin")) {
        if (p.path().filename() == "UserScripts.dll") dllPath = p.path();
        if (p.path().filename() == "UserScripts.runtimeconfig.json") configPath = p.path();
    }

    if (dllPath.empty()) {
        std::cerr << "[Editor] Could not find built artifacts (UserScripts.dll) in " << scriptFolder << "/bin" << std::endl;
        return false;
    }

    try {
        std::cout << "[Editor] Copying " << dllPath.filename() << " to " << exeDir << std::endl;
        fs::copy_file(dllPath, exeDir / "UserScripts.dll", fs::copy_options::overwrite_existing);
        // We don't strictly need the runtimeconfig for UserScripts anymore since Loader handles it, but good to have.
        if (!configPath.empty()) {
             fs::copy_file(configPath, exeDir / "UserScripts.runtimeconfig.json", fs::copy_options::overwrite_existing);
        }
    } catch (std::exception& e) {
        std::cerr << "[Editor] Failed to copy script artifacts: " << e.what() << std::endl;
        return false;
    }

    // 4. Init Engine
    // We pass exeDir because that's where we copied the DLLs
    ScriptEngine::Init(exeDir.string().c_str());
    RunStarts();

    return true;
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
    
    // Check for Dev Environment (Source Scripts)
    bool devEnv = false;
#if defined(__APPLE__)
    if (exeDir.filename() == "MacOS" && exeDir.parent_path().filename() == "Contents") {
        fs::path potentialRoot = exeDir.parent_path().parent_path().parent_path().parent_path(); // .../Stela
        fs::path sourceScripts = potentialRoot / "Scripts" / "DotNet" / "UserScripts"; // Source location
        
        if (fs::exists(sourceScripts) && fs::exists(sourceScripts / "UserScripts.csproj")) {
            scriptFolder = sourceScripts;
            devEnv = true;
            std::cout << "[Editor] Development Environment Detected. Using Source Scripts at: " << scriptFolder << std::endl;
        }
    }
#endif

    // Ensure we are working in the correct directory (fixes relative path issues)
    fs::current_path(exeDir);
    
    // Add common locations for dotnet to PATH (GUI apps often have limited PATH)
    #if defined(__APPLE__) || defined(__linux__)
    std::string pathEnv = std::getenv("PATH");
    std::string newPath = pathEnv + ":/usr/local/bin:/usr/local/share/dotnet:/opt/homebrew/bin"; // Standard locations
    setenv("PATH", newPath.c_str(), 1);
    #endif

    if (!devEnv) {
        scriptFolder = exeDir / "Scripts";
    }

    if (!fs::exists(scriptFolder)) {
        std::cerr << "[Editor] Warning: 'Scripts' folder not found next to executable (" << scriptFolder << ")\n";
    }

    // Initial Load
    // Only build if source changed or DLL missing
    bool dllExists = fs::exists(exeDir / "UserScripts.dll");
    
    // Initialize lastScriptWriteTime from existing DLL if possible
    if (dllExists) {
        lastScriptWriteTime = fs::last_write_time(exeDir / "UserScripts.dll");
    } else {
        lastScriptWriteTime = fs::file_time_type::min();
    }

    if (fs::exists(scriptFolder)) {
        // Check if we need to build
        if (!dllExists || ScriptsChanged()) {
            if (!ReloadScripts(&engine)) {
                std::cerr << "[Editor] Initial script load failed.\n";
            }
        } else {
            // DLL exists and is newer than source, just load it
            std::cout << "[Editor] Scripts up to date. Loading existing UserScripts.dll...\n";
            ScriptEngine::Init(exeDir.string().c_str());
            RunStarts();
        }
    } else {
        std::cerr << "[Editor] No scripts to load.\n";
    }

    // Start watcher thread
    std::thread watcher([&]()
    {
        while (!quit)
        {
            if (ScriptsChanged())
            {
                // We use a flag to signal the main thread
                // Since ScriptsChanged() updates lastScriptWriteTime, we don't need to do it here
                // But wait, ScriptsChanged() returns true if latest > lastScriptWriteTime
                // and UPDATES lastScriptWriteTime.
                // So if we call it here and it returns true, the state is updated.
                // We just need to tell main thread to Reload.
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // check every 500ms
        }
    });

    // We can't run ReloadScripts in a separate thread easily if it touches engine state that isn't thread safe.
    // ScriptEngine::Init registers callbacks into gScriptSystems (std::vector).
    // The engine loop iterates gScriptSystems.
    // So we MUST NOT modify gScriptSystems while engine is running.
    // That's why we pause.
    
    // Better approach: watcher sets a flag, main loop handles reload.
    bool pendingReload = false;
    // Persistent UI toggles
    bool showFPSWindow = false;

        while (!quit)
    {
        // Check watcher result (simplified for now: watcher just sleeps, we check here? No, watcher is better for file I/O)
        // Let's move file checking to main loop to avoid threading issues for now, or just use atomic flag.
        if (ScriptsChanged()) {
            pendingReload = true;
        }

        if (pendingReload) {
            // Wait for end of frame? We are at start of frame.
            ReloadScripts(&engine);
            pendingReload = false;
        }

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

        // 1. Menu Bar (must be first to update Viewport WorkArea)
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    quit = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Scripts")) {
                if (ImGui::MenuItem("Reload Scripts")) {
                    ReloadScripts(&engine);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                // Toggle persistent FPS window instead of creating it transiently inside the menu
                ImGui::MenuItem("FPS", nullptr, &showFPSWindow);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        static bool first_time = true;
        if (first_time) {
            first_time = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
            
            ImGuiID dock_main_id = dockspace_id;
            ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
            
            ImGui::DockBuilderFinish(dockspace_id);
        }
        ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Viewport Window
        ImGui::Begin("Viewport");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
#if !defined(__APPLE__)
        static VkDescriptorSet sceneDS = VK_NULL_HANDLE;
        if (sceneDS == VK_NULL_HANDLE) {
             sceneDS = ImGui_ImplVulkan_AddTexture(engine.vulkan.OffscreenSampler, engine.vulkan.OffscreenImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        ImGui::Image((ImTextureID)sceneDS, viewportSize);
#else
        ImGui::Image((ImTextureID)engine.metal.OffscreenTexture, viewportSize);
#endif
        ImGui::End();

        // Persistent FPS window (stays open until user closes it)
        if (showFPSWindow) {
            ImGui::Begin("Debug: FPS", &showFPSWindow);
            ImGui::Text("FPS: %.1f", 1.0f / engine.deltaTime);
            ImGui::End();
        }

        ImGui::Render();

        engine.RunFrame();
        if (engine.bQuit)
            quit = true;
    }

    watcher.detach(); // Allow it to die

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
    ScriptEngine::Shutdown();
    return 0;
}
