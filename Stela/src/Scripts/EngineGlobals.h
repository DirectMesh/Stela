#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
  #if defined(Stela_EXPORTS)
    #define STELA_API __declspec(dllexport)
  #else
    #define STELA_API __declspec(dllimport)
  #endif
#else
  #define STELA_API
#endif

struct ScriptSystem
{
    std::string name;
    void (*Start)();
    void (*Update)(float);
    void (*Shutdown)();
};

// One single global vector, no static
extern STELA_API std::vector<ScriptSystem> gScriptSystems;
