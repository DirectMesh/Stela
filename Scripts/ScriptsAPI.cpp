#include <Scripts/ScriptsAPI.h>
#include <Scripts/EngineGlobals.h>
#include <Input/Input.h>
#include <cstdio>
#include <iostream>

static ScriptsAPI* gAPI = nullptr;

static void PlayerSystem(float dt)
{
    if (gAPI && gAPI->Log) {
        if (Input::KeyPressed(Input::W)) {
            gAPI->Log("PlayerSystem tick");
        }
        if (Input::KeyPressed(Input::Space)) {
            gAPI->Log("Player jumped!");
        }
        if (Input::KeyDown(Input::A)) {
            gAPI->Log("Player moving left");
        }
        if (Input::KeyReleased(Input::D)) {
            gAPI->Log("Player stopped moving right");
        }
        if (Input::GamepadButtonPressed(0, Input::GP_A)) {
            gAPI->Log("Gamepad A button pressed");
            Input::SetGamepadVibration(0, Input::GP_VibrationLeft, 0.75f);
        }
    }
    else
        std::cout << "[PlayerSystem] gAPI null!" << std::endl;
}

SCRIPTS_API void Scripts_Init(ScriptsAPI* api)
{
    if (api->Version != 1.0)
    {
        api->Log("Scripts API version mismatch!");
        return;
    }

    gAPI = api;

    if (gAPI && gAPI->Log)
    {
        gAPI->Log("Scripts.dylib successfully loaded!");
    }

    api->RegisterSystem("PlayerSystem", PlayerSystem);
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
    // Clear API pointer
    gAPI = nullptr;
}
