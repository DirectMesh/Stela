#pragma once

#if defined(_WIN32)
    #if defined(SCRIPTS_EXPORTS)
        #define SCRIPTS_API extern "C" __declspec(dllexport)
    #else
        #define SCRIPTS_API extern "C" __declspec(dllimport)
    #endif
#else
    #define SCRIPTS_API extern "C"
#endif

struct ScriptsAPI
{
    int Version;
    void (*Log)(const char* message);
    void (*RegisterSystem)(const char* name, void (*updateFn)(float));
};

// Mandatory entry point
SCRIPTS_API void Scripts_Init(ScriptsAPI* api);
SCRIPTS_API void Scripts_Shutdown();
