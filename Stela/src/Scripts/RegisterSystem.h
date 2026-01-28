#pragma once
#include "EngineGlobals.h"
#include <vector>
#include <string>
#include <iostream>

inline void Engine_RegisterScript(const char* name, void (*start)(), void (*update)(float), void (*shutdown)())
{
    gScriptSystems.push_back({ name, start, update, shutdown });
    std::cout << "[Engine] Registered script: " << name << std::endl;
}

inline void RunStarts()
{
    for (auto& sys : gScriptSystems) {
        if (sys.Start) {
            sys.Start();
        }
    }
}

inline void RunSystems(float dt)
{
    for (auto& sys : gScriptSystems) {
        if (sys.Update) {
            sys.Update(dt);
        }
    }
}

inline void RunShutdowns()
{
    for (auto& sys : gScriptSystems) {
        if (sys.Shutdown) {
            sys.Shutdown();
        }
    }
}
