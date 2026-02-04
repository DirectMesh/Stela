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

// Simple build and copy logic for Runtime startup
bool BuildAndCopyScripts(const fs::path& scriptFolder, const fs::path& exeDir)
{
    std::cout << "[Runtime] Building Scripts...\n";
    std::string buildCmd = "cd \"" + scriptFolder.string() + "\" && dotnet build -c Debug";
    
    if (system(buildCmd.c_str()) != 0)
    {
        std::cerr << "[Runtime] dotnet build failed.\n";
        return false;
    }

    fs::path dllPath;
    fs::path configPath;
    
    if (fs::exists(scriptFolder / "bin")) {
        for(auto& p : fs::recursive_directory_iterator(scriptFolder / "bin")) {
            if (p.path().filename() == "UserScripts.dll") dllPath = p.path();
            if (p.path().filename() == "UserScripts.runtimeconfig.json") configPath = p.path();
        }
    }

    if (dllPath.empty() || configPath.empty()) {
        std::cerr << "[Runtime] Could not find built artifacts.\n";
        return false;
    }

    try {
        fs::copy_file(dllPath, exeDir / "UserScripts.dll", fs::copy_options::overwrite_existing);
        fs::copy_file(configPath, exeDir / "UserScripts.runtimeconfig.json", fs::copy_options::overwrite_existing);
        return true;
    } catch (std::exception& e) {
        std::cerr << "[Runtime] Failed to copy script artifacts: " << e.what() << std::endl;
        return false;
    }
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
    
    // Check if we need to build scripts
    // If UserScripts.dll is missing OR if Scripts folder exists (we might want to ensure latest is built)
    // For Runtime, maybe we only build if DLL is missing? 
    // Or if the user provided source, they probably expect it to run.
    bool dllExists = fs::exists(exeDir / "UserScripts.dll");
    bool sourceExists = fs::exists(scriptFolder);

    if (sourceExists) {
        // If source exists, let's try to build to ensure we run the code provided in Scripts folder
        // This matches the "drop scripts folder and run" workflow.
        if (!BuildAndCopyScripts(scriptFolder, exeDir)) {
            std::cerr << "[Runtime] Failed to build scripts from " << scriptFolder << "\n";
            // If build failed, maybe try running existing DLL if available?
            if (dllExists) {
                std::cout << "[Runtime] Falling back to existing UserScripts.dll\n";
            }
        }
    } else if (!dllExists) {
        std::cerr << "[Runtime] No 'Scripts' folder and no 'UserScripts.dll' found.\n";
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
