    #pragma once
    #include "EngineGlobals.h"
    #include <vector>
    #include <string>
    #include <iostream>

    void Engine_RegisterSystem(const char* name, void (*updateFn)(float))
    {
        gScriptSystems.push_back({ name, updateFn });

        std::cout << "[Engine] Registered system: " << name << std::endl;
    }

void RunSystems(float dt)
{
    if (gScriptSystems.empty()) {
        std::cout << "error" << std::endl;
        return;
    }
    for (auto& sys : gScriptSystems) {
        if (sys.Update) {
            sys.Update(dt);
        } else {
            std::cout << "[Engine] Warning: system '" << sys.name << "' has null Update" << std::endl;
        }
    }
}
