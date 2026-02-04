#include <Scripts/ScriptsAPI.h>
#include <Scripts/EngineGlobals.h>
#include "DotNetHost.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

// This is the ONE definition of gAPI
ScriptsAPI* gAPI = nullptr;

std::string GetLibraryPath() {
#if defined(__APPLE__) || defined(__linux__)
    Dl_info info;
    if (dladdr((void*)Scripts_Init, &info)) {
        return fs::path(info.dli_fname).parent_path().string();
    }
#endif
    return ".";
}

void DotNetStart() {
    // C# scripts are loaded and started in DotNetHost::Init currently
}

void DotNetUpdate(float dt) {
    DotNetHost::Update(dt);
}

void DotNetShutdownScript() {
    // Called when the system shuts down
}

SCRIPTS_API void Scripts_Init(ScriptsAPI* api)
{
    if (api->Version != 1)
    {
        if (api->Log)
            api->Log("Scripts API version mismatch!");
        return;
    }

    gAPI = api;

    if (gAPI && gAPI->Log)
    {
        gAPI->Log("Scripts successfully loaded (C# Support)!");
    }

    std::string libPath = GetLibraryPath();
    if (DotNetHost::Init(libPath.c_str())) {
        if (gAPI && gAPI->Log) gAPI->Log("DotNet Host Initialized.");
        
        // Register the DotNet bridge script
        api->RegisterScript("DotNetRuntime", DotNetStart, DotNetUpdate, DotNetShutdownScript);
        
    } else {
        if (gAPI && gAPI->Log) gAPI->Log("Failed to initialize DotNet Host.");
    }
}

SCRIPTS_API void Scripts_Shutdown()
{
    DotNetHost::Shutdown();

    if (!gAPI)
        return;

    gAPI->Log("Scripts shutting down");
    gAPI = nullptr; // clear global pointer
}
