#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

struct ScriptSystem
{
    std::string name;
    void (*Update)(float);
};

// One single global vector, no static
extern std::vector<ScriptSystem> gScriptSystems;
