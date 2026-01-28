#pragma once
#include <Scripts/ScriptsAPI.h>
#include <Scripts/EngineGlobals.h>

// Declare life-cycle functions: OnStart runs once at script start,
// OnUpdate runs every frame with delta time, OnShutdown runs at script shutdown.
OnStart(Example);
OnUpdate(Example2);
OnShutdown(ExampleShutdown);