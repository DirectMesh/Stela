#include <Stela.h>
#include <iostream>

int main() {
    Stela engine;

    engine.Init("Stela Editor", 1920, 1080);
    engine.Run();
    engine.Cleanup();

    return 0;
}