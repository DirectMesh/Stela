#include <Stela.h>
#include <iostream>

int main() {
    Stela engine;

    engine.Init();
    engine.Run();
    engine.Cleanup();

    return 0;
}