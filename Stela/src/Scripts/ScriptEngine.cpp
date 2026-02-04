#include "DotNetHost.h"
#include "RegisterSystem.h"
#include <iostream>
#include <string>
#include <filesystem>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

// Internal helpers to register the system
void DotNetStart() {
    // C# scripts are loaded and started in DotNetHost::Init currently
}

void DotNetUpdate(float dt) {
    DotNetHost::Update(dt);
}

void DotNetShutdownScript() {
    // Called when the system shuts down
}

namespace ScriptEngine {

    void Init(const char* assemblyDir) {
        if (DotNetHost::Init(assemblyDir)) {
            std::cout << "[ScriptEngine] DotNet Host Initialized." << std::endl;
            
            // Register the DotNet bridge script
            Engine_RegisterScript("DotNetRuntime", DotNetStart, DotNetUpdate, DotNetShutdownScript);
            
        } else {
            std::cerr << "[ScriptEngine] Failed to initialize DotNet Host." << std::endl;
        }
    }

    void Shutdown() {
        DotNetHost::Shutdown();
    }
}