#include "Example.h"
#include <Input/Input.h>
#include <iostream>

extern ScriptsAPI* gAPI;

void Example()
{
    if (gAPI && gAPI->Log) {
        gAPI->Log("Example script started!");
    } else {
        std::cout << "[Example] Start: gAPI null!" << std::endl;
    }
}

void Example2(float dt)
{
    if (gAPI && gAPI->Log) {
        if (Input::KeyPressed(Input::S)) {
            gAPI->Log("This Is An Example (key pressed)");
        }
    } else {
        std::cout << "[Example] Update: gAPI null!" << std::endl;
    }
}

void ExampleShutdown()
{
    if (gAPI && gAPI->Log) {
        gAPI->Log("Example script shutting down.");
    } else {
        std::cout << "[Example] Shutdown: gAPI null!" << std::endl;
    }
}
