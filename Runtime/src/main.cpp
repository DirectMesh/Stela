#include <Stela.h>
#include <Scripts/ScriptEngine.h>
#include <Scripts/RegisterSystem.h>
#include <Scripts/EngineGlobals.h>

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

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

void EngineLog(const char* msg)
{
    std::cout << "[Stela] " << msg << std::endl;
}

int main()
{
    Stela engine;
    engine.Init("Stela Runtime");

    auto exeDir = GetExeDir();
    
#if defined(__APPLE__)
    if (exeDir.filename() == "MacOS" && exeDir.parent_path().filename() == "Contents") {
        exeDir = exeDir.parent_path().parent_path().parent_path();
        std::cout << "[Runtime] Running in Bundle. Setting Root Directory to: " << exeDir << std::endl;
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

    fs::path scriptFolder = exeDir / "Scripts";
    
    // Runtime should not build scripts. It expects UserScripts.dll to be present.
    bool dllExists = fs::exists(exeDir / "UserScripts.dll");

    if (!dllExists) {
        std::cerr << "[Runtime] Error: 'UserScripts.dll' not found. Please build the project using the Editor.\n";
    }

    // Init Engine
    ScriptEngine::Init(exeDir.string().c_str());
    RunStarts();

    engine.Run();

    engine.Cleanup();
    
    RunShutdowns();
    ScriptEngine::Shutdown();

    return 0;
}
