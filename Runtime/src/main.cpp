#include <Stela.h>
#include <DynamicLibrary.h>
#include <Scripts/ScriptsAPI.h>
#include <Scripts/RegisterSystem.h>
#include <iostream>
#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

static std::filesystem::path GetExeDir()
{
#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    _NSGetExecutablePath(path.data(), &size);
    return std::filesystem::path(path).parent_path();

#elif defined(__linux__)
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
    return std::filesystem::path(std::string(buffer, len)).parent_path();
#endif
}

#if defined(_WIN32)
const char *ext = ".dll";
#elif defined(__APPLE__)
const char *ext = ".dylib";
#else
const char *ext = ".so";
#endif

static ScriptsAPI gEngineAPI{};
static bool ValidateRegisteredSystems()
{
#if defined(__APPLE__) || defined(__linux__)
    Dl_info info{};
    for (auto &sys : gScriptSystems)
    {
        void *fn = (void *)sys.Update ? (void *)sys.Update : sys.Start ? (void *)sys.Start : (void *)sys.Shutdown;
        if (!fn)
        {
            std::cerr << "[Runtime] Invalid script '" << sys.name << "': no callbacks\n";
            return false;
        }

        if (dladdr(fn, &info) == 0)
        {
            std::cerr << "[Runtime] dladdr failed for script '" << sys.name << "'\n";
            return false;
        }

        if (!info.dli_fname)
        {
            std::cerr << "[Runtime] dladdr returned null fname for script '" << sys.name << "'\n";
            return false;
        }

        std::string libpath(info.dli_fname);
        std::string expected = std::string("Scripts") + ext;

        if (libpath.find(expected) == std::string::npos)
        {
            std::cerr << "[Runtime] Script '" << sys.name
                      << "' points to unexpected image: " << libpath << "\n";
            return false;
        }
    }
#endif
    return true;
}

void *scriptModule = nullptr;
void (*initFn)(ScriptsAPI *) = nullptr;
void (*shutdownFn)() = nullptr;

std::string scriptSourceFolder = "./Scripts";
std::string scriptLibPath;

void EngineLog(const char *msg)
{
    std::cout << "[Stela] " << msg << std::endl;
}

int main()
{
    Stela engine;

    engine.Init();

    auto exeDir = GetExeDir();

    std::string libName = std::string("Scripts") + ext;

    std::filesystem::path candidates[] = {
        exeDir / libName,
        exeDir / "bin" / libName};

    bool found = false;
    for (auto &p : candidates)
    {
        if (std::filesystem::exists(p))
        {
            scriptLibPath = p.string();
            found = true;
            break;
        }
    }

    if (!found)
    {
        std::cerr << "[Editor] Failed to find Scripts library\n";
        for (auto &p : candidates)
            std::cerr << "  Tried: " << p << "\n";
        return 1;
    }

    // Load initial Scripts library
    scriptModule = DynamicLibrary::Load(scriptLibPath.c_str());
    if (!scriptModule)
    {
        std::cerr << "[Editor] Failed to load " << scriptLibPath << std::endl;
        return 1;
    }

    initFn = (void (*)(ScriptsAPI *))DynamicLibrary::GetSymbol(scriptModule, "Scripts_Init");
    shutdownFn = (void (*)())DynamicLibrary::GetSymbol(scriptModule, "Scripts_Shutdown");

    if (!initFn || !shutdownFn)
    {
        std::cerr << "[Editor] Failed to find Scripts_Init or Scripts_Shutdown\n";
        return 1;
    }

    gEngineAPI.Version = 1;
    gEngineAPI.Log = EngineLog;
    gEngineAPI.RegisterScript = Engine_RegisterScript;
    initFn(&gEngineAPI);
    RunStarts();
    if (!ValidateRegisteredSystems())
    {
        std::cerr << "[Editor] Registered systems validation failed on initial load\n";
        return 1;
    }
    engine.Run();
    engine.Cleanup();

    if (scriptModule)
    {
        RunShutdowns();
        shutdownFn();
        DynamicLibrary::Unload(scriptModule);
    }

    return 0;
}