#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

struct ScriptSystem
{
    std::string name;
    void (*Start)();
    void (*Update)(float);
    void (*Shutdown)();
};

// One single global vector, no static
extern std::vector<ScriptSystem> gScriptSystems;
