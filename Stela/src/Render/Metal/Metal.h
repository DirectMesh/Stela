#pragma once

#if defined(__APPLE__)

namespace MTL { class Device; }

class Metal {
public:
    
    MTL::Device* Device;
    void Init();
    void CreateDevice();
    void Cleanup();
};

#endif
