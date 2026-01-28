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

    exeDir = GetExeDir();
    std::string libName = std::string("Scripts") + ext;

    // Scripts source folder
    for (auto& p : { exeDir / "Scripts", exeDir.parent_path() / "Scripts" })
    {
        if (fs::exists(p))
            scriptSourceFolder = p;
    }

    // Compiled library
    for (auto& p : { exeDir / libName, exeDir / "bin" / libName })
    {
        if (fs::exists(p))
            scriptBuildLibPath = p;
    }

    // CMake build directory
    for (auto& p : {
             exeDir / "build",
             exeDir.parent_path() / "build",
             exeDir.parent_path().parent_path() / "build"
         })
    {
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
        engine.RunFrame();
        if (engine.bQuit)
            quit = true;
    }

    watcher.join();

    engine.Cleanup();
    RunShutdowns();
    shutdownFn();
    DynamicLibrary::Unload(scriptModule);
    return 0;
}
