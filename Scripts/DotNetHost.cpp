#include "DotNetHost.h"
#include <Scripts/ScriptsAPI.h>
#include <Input/Input.h>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <iostream>
#include <filesystem>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#define STR(s) L##s
#define CH(c) L##c
#define DIR_SEPARATOR L'\\'
#else
#include <dlfcn.h>
#include <limits.h>
#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX
typedef char char_t;
#endif

// Define UNMANAGEDCALLERSONLY_METHOD if not defined by coreclr_delegates.h
#ifndef UNMANAGEDCALLERSONLY_METHOD
#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)
#endif

namespace fs = std::filesystem;

extern ScriptsAPI* gAPI;

namespace DotNetHost {

    // Globals to hold delegates
    // Init now takes: LogCallback, KeyPressedCallback
    void (*csharp_init)(void (*)(const char*), bool (*)(int)) = nullptr;
    void (*csharp_update)(float) = nullptr;
    void (*csharp_shutdown)() = nullptr;

    hostfxr_initialize_for_runtime_config_fn init_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;

    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

    void* LoadLibrary(const char_t* path) {
#if defined(_WIN32)
        HMODULE h = ::LoadLibraryW(path);
        return (void*)h;
#else
        void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        return h;
#endif
    }

    void* GetExport(void* h, const char* name) {
#if defined(_WIN32)
        return (void*)::GetProcAddress((HMODULE)h, name);
#else
        return dlsym(h, name);
#endif
    }

    bool LoadHostFxr() {
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
        if (rc != 0) return false;

        void* lib = LoadLibrary(buffer);
        if (!lib) return false;

        init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetExport(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetExport(lib, "hostfxr_get_runtime_delegate");
        close_fptr = (hostfxr_close_fn)GetExport(lib, "hostfxr_close");

        return (init_fptr && get_delegate_fptr && close_fptr);
    }

    // Callback passed to C#
    void LogCallback(const char* msg) {
        if (gAPI && gAPI->Log) {
            gAPI->Log(msg);
        } else {
            std::cout << "[DotNet] " << msg << std::endl;
        }
    }

    bool DotNetInput_KeyPressed(int key) {
        return Input::KeyPressed((Input::Keys)key);
    }

    bool Init(const char* assemblyDir) {
        if (!LoadHostFxr()) {
            std::cerr << "Failed to load hostfxr" << std::endl;
            return false;
        }

        fs::path dir(assemblyDir);
        fs::path configPath = dir / "UserScripts.runtimeconfig.json";
        fs::path dllPath = dir / "UserScripts.dll";

        if (!fs::exists(configPath) || !fs::exists(dllPath)) {
            std::cerr << "Files missing: " << configPath << " or " << dllPath << std::endl;
            return false;
        }

        hostfxr_handle cxt = nullptr;
        int rc = init_fptr(configPath.c_str(), nullptr, &cxt);
        if (rc != 0 && rc != 1) { // 1 = Host already initialized
            std::cerr << "Init failed: " << std::hex << rc << std::endl;
            if (cxt) close_fptr(cxt);
            return false;
        }

        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            (void**)&load_assembly_and_get_function_pointer);
        
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr) {
            std::cerr << "Get delegate failed: " << std::hex << rc << std::endl;
            if (cxt) close_fptr(cxt);
            return false;
        }

        if (cxt) close_fptr(cxt); // Initialization complete

        // Load C# methods
        const char_t* dotnet_type = STR("Stela.ScriptManager, UserScripts");
        const char_t* dotnet_type_method_init = STR("Init");
        const char_t* dotnet_type_method_update = STR("Update");
        const char_t* dotnet_type_method_shutdown = STR("Shutdown");

        // Init
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            dotnet_type,
            dotnet_type_method_init,
            UNMANAGEDCALLERSONLY_METHOD, // or nullptr for default if compatible? No, we need delegate type or UnmanagedCallersOnly
            // Wait, standard delegate lookup:
            // "public static void Init(IntPtr)"
            // signature delegate type name:
            nullptr, // use default signature matching
            (void**)&csharp_init);

        if (rc != 0) {
             std::cerr << "Failed to get Init: " << std::hex << rc << std::endl;
             // Try specifying delegate type if needed, but simple signature usually works with default
        }

        // Update
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            dotnet_type,
            dotnet_type_method_update,
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&csharp_update);

        if (rc != 0) {
            std::cerr << "Failed to get Update: " << std::hex << rc << std::endl;
        }

        // Shutdown
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            dotnet_type,
            dotnet_type_method_shutdown,
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&csharp_shutdown);

        if (rc != 0) {
            std::cerr << "Failed to get Shutdown: " << std::hex << rc << std::endl;
        }

        if (csharp_init) {
            csharp_init(LogCallback, DotNetInput_KeyPressed);
            return true;
        }

        return false;
    }

    void Shutdown() {
        if (csharp_shutdown) csharp_shutdown();
    }

    void Update(float dt) {
        if (csharp_update) csharp_update(dt);
    }
}
