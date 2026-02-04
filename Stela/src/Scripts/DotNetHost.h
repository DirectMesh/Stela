#pragma once

namespace DotNetHost {
    bool Init(const char* assemblyDir);
    void Shutdown();
    void Update(float dt);
}