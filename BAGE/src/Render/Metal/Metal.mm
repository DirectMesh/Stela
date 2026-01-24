#if defined(__APPLE__)

#include "Metal.h"
#include <iostream>
#include <Metal/Metal.hpp>

void Metal::Init() {
    CreateDevice();
}

void Metal::CreateDevice() {
    Device = MTL::CreateSystemDefaultDevice(); // <— assign to the member variable
    if (!Device) {
        std::cerr << "Failed to create Metal device!" << std::endl;
    }
    
    std::cout << "Metal device: " << Device->name()->utf8String() << std::endl;
}

void Metal::Cleanup() {
    if (Device) {
        Device->release();
        Device = nullptr;
    }
}

#endif
