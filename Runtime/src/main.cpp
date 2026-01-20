#include <BAGE.h>
#include <iostream>

int main() {
    BAGE engine;

    engine.Init();
    engine.Run();
    engine.Cleanup();

    return 0;
}