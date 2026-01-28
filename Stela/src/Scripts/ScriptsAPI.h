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

// Declaration macros for script life-cycle functions
#define OnStart(FuncName)   void FuncName()
#define OnUpdate(FuncName)  void FuncName(float dt)
#define OnShutdown(FuncName) void FuncName()

struct ScriptsAPI
{
    int Version;
    void (*Log)(const char* message);
    void (*RegisterScript)(const char* name, void (*start)(), void (*update)(float), void (*shutdown)());
};

// Mandatory entry point
SCRIPTS_API void Scripts_Init(ScriptsAPI* api);
SCRIPTS_API void Scripts_Shutdown();
