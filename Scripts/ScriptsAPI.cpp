#include <Scripts/ScriptsAPI.h>
#include <Scripts/EngineGlobals.h>
#include "Engine/Example.h"
#include <cstdio>
#include <iostream>

// This is the ONE definition of gAPI
ScriptsAPI* gAPI = nullptr;

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
        gAPI->Log("Scripts successfully loaded!");
    }

    // Register scripts: name, OnStart, OnUpdate, OnShutdown (any can be nullptr)
    api->RegisterScript("Example", Example, Example2, ExampleShutdown);
}

SCRIPTS_API void Scripts_Shutdown()
{
    if (!gAPI)
        return;

    gAPI->Log("Scripts shutting down");

    // Clear all registered script systems
    for (auto& sys : gScriptSystems) {
        gAPI->Log(("Removing system: " + sys.name).c_str());
    }
    gScriptSystems.clear();

    gAPI = nullptr; // clear global pointer
}