#include <BAGE.h>
#include <iostream>

int main() {
    BAGE engine;

    engine.Init("BAGE Editor", 1920, 1080);
    engine.Run();
    engine.Cleanup();

    return 0;
}