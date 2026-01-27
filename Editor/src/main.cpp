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
#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

#if defined(_WIN32)
const char* ext = ".dll";
#elif defined(__APPLE__)
const char* ext = ".dylib";
#else
const char* ext = ".so";
#endif

std::atomic<bool> quit{false};
extern std::atomic<bool> enginePaused;

static ScriptsAPI gEngineAPI{};
static bool ValidateRegisteredSystems()
{
#if defined(__APPLE__) || defined(__linux__)
    Dl_info info{};
    for (auto& sys : gScriptSystems) {
        if (!sys.Update) {
            std::cerr << "[Editor] Invalid system '" << sys.name << "': null Update pointer\n";
            return false;
        }

        if (dladdr((void*)sys.Update, &info) == 0) {
            std::cerr << "[Editor] dladdr failed for system '" << sys.name << "'\n";
            return false;
        }

        if (!info.dli_fname) {
            std::cerr << "[Editor] dladdr returned null fname for system '" << sys.name << "'\n";
            return false;
        }

        std::string libpath(info.dli_fname);

        // Build expected script library name using ext
        std::string expected = std::string("Scripts") + ext;

        if (libpath.find(expected) == std::string::npos) {
            std::cerr << "[Editor] System '" << sys.name
                      << "' points to unexpected image: " << libpath << "\n";
            return false;
        }
    }
#endif
    return true;
}

// Global pointers to the current script module & functions
void* scriptModule = nullptr;
void (*initFn)(ScriptsAPI*) = nullptr;
void (*shutdownFn)() = nullptr;
fs::file_time_type lastScriptWriteTime;

// Paths
std::string scriptSourceFolder = "./Scripts";
std::string scriptLibPath;

// Logging function
void EngineLog(const char* msg) {
    std::cout << "[Stela] " << msg << std::endl;
}

// Check if Scripts folder changed
bool ScriptsChanged() {
    try {
        auto latestWrite = fs::file_time_type::min();
        for (auto& p : fs::recursive_directory_iterator(scriptSourceFolder)) {
            if (p.is_regular_file()) {
                auto writeTime = fs::last_write_time(p.path());
                if (writeTime > latestWrite) latestWrite = writeTime;
            }
        }

        if (latestWrite != lastScriptWriteTime) {
            lastScriptWriteTime = latestWrite;
            return true;
        }
    } catch (fs::filesystem_error& e) {
        std::cerr << "[Editor] Error checking Scripts folder: " << e.what() << std::endl;
    }
    return false;
}

// Reload Scripts DLL
bool ReloadScripts(Stela* engine) {
    if (scriptModule) {
        shutdownFn();

        DynamicLibrary::Unload(scriptModule);
        scriptModule = nullptr;
    }

    std::cout << "[Editor] Building Scripts...\n";
    int result = system("cmake --build build --target Scripts");
    if (result != 0) {
        std::cerr << "[Editor] Scripts build failed with code " << result << std::endl;
        SDL_MessageBoxData data{};
        data.flags = SDL_MESSAGEBOX_ERROR;
        data.title = "Scripts Reload Failed";
        data.message = "Building Scripts failed.\nCheck console output.";
        data.window = engine->Window;
        data.numbuttons = 1;

        SDL_MessageBoxButtonData button{};
        button.buttonID = 0;
        button.text = "OK";

        data.buttons = &button;

        SDL_ShowMessageBox(&data, nullptr);
        return false;
    }

    scriptModule = DynamicLibrary::Load(scriptLibPath.c_str());
    if (!scriptModule) {
        std::cerr << "[Editor] Failed to load Scripts library!\n";
        return false;
    }

    initFn = (void(*)(ScriptsAPI*))DynamicLibrary::GetSymbol(scriptModule, "Scripts_Init");
    shutdownFn = (void(*)())DynamicLibrary::GetSymbol(scriptModule, "Scripts_Shutdown");

    if (!initFn || !shutdownFn) {
        std::cerr << "[Editor] Failed to find Scripts_Init or Scripts_Shutdown\n";
        return false;
    }

    gEngineAPI.Version = 1;
    gEngineAPI.Log = EngineLog;
    gEngineAPI.RegisterSystem = Engine_RegisterSystem;

    initFn(&gEngineAPI);
    if (!ValidateRegisteredSystems()) {
        std::cerr << "[Editor] Registered systems validation failed after reload\n";
        return false;
    }
    std::cout << "[Editor] Scripts reloaded successfully.\n";
    return true;
}

int main() {
    // Initialize Engine (SDL runs on main thread)
    Stela engine;
    engine.Init("Stela Editor", 1920, 1080);

    //Determine compiled library path
#if defined(_WIN32)
    scriptLibPath = "./bin/Scripts.dll";
#elif defined(__APPLE__)
    scriptLibPath = "./bin/Scripts.dylib";
#else
    scriptLibPath = "./bin/Scripts.so";
#endif

    // Load initial Scripts library
    scriptModule = DynamicLibrary::Load(scriptLibPath.c_str());
    if (!scriptModule) {
        std::cerr << "[Editor] Failed to load " << scriptLibPath << std::endl;
        return 1;
    }

    initFn = (void(*)(ScriptsAPI*))DynamicLibrary::GetSymbol(scriptModule, "Scripts_Init");
    shutdownFn = (void(*)())DynamicLibrary::GetSymbol(scriptModule, "Scripts_Shutdown");

    if (!initFn || !shutdownFn) {
        std::cerr << "[Editor] Failed to find Scripts_Init or Scripts_Shutdown\n";
        return 1;
    }

    gEngineAPI.Version = 1;
    gEngineAPI.Log = EngineLog;
    gEngineAPI.RegisterSystem = Engine_RegisterSystem;
    initFn(&gEngineAPI);
    if (!ValidateRegisteredSystems()) {
        std::cerr << "[Editor] Registered systems validation failed on initial load\n";
        return 1;
    }

    // Record initial last write time
    try {
        auto latestWrite = fs::file_time_type::min();
        for (auto& p : fs::recursive_directory_iterator(scriptSourceFolder)) {
            if (p.is_regular_file()) {
                auto writeTime = fs::last_write_time(p.path());
                if (writeTime > latestWrite) latestWrite = writeTime;
            }
        }
        lastScriptWriteTime = latestWrite;
    } catch (fs::filesystem_error& e) {
        std::cerr << "[Editor] Error accessing Scripts folder: " << e.what() << std::endl;
    }

    // Background thread: watch scripts folder
    std::thread watcher([&engine]() {
        while (!quit) {
            if (ScriptsChanged()) {
                engine.bPauseRun = true;
                while (!enginePaused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
                if (ReloadScripts(&engine)) {
                    engine.bPauseRun = false;
                    while (enginePaused) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                } else {
                    std::cout << "[Editor] Reload failed; engine remains paused. Watching for further changes..." << std::endl;
                }
            }
        }
    });

    // Main loop (SDL on main thread!)
    while (!quit) {
        engine.RunFrame();  // safe, SDL calls only here

        // Optional: break loop on SDL quit
        if (engine.bQuit) {
            quit = true;
        }
    }

    watcher.join();

    engine.Cleanup();

    if (scriptModule) {
        shutdownFn();
        DynamicLibrary::Unload(scriptModule);
    }

    return 0;
}
