#if defined(__APPLE__)

#include "Metal.h"
#include <iostream>
#include <Metal/Metal.hpp>
#include <SDL3/SDL.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <mach-o/dyld.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>

void Metal::Init(SDL_Window* window) {
    CreateDevice();
    metalWindow = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    metalLayer = [CAMetalLayer layer];
    metalLayer.device = (__bridge id<MTLDevice>)Device;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalWindow.contentView.layer = metalLayer;
    metalWindow.contentView.wantsLayer = YES;
    metalLayer.drawableSize = CGSizeMake(width, height);
    createTriangle();
    createDefaultLibrary();
    createCommandQueue();
    createRenderPipeline();
}

void Metal::CreateDevice() {
    Device = MTL::CreateSystemDefaultDevice(); // <— assign to the member variable
    if (!Device) {
        std::cerr << "Failed to create Metal device!" << std::endl;
    }
    
    std::cout << "Metal device: " << Device->name()->utf8String() << std::endl;
}

void Metal::createTriangle() {
    // Metal NDC: X right, Y up, Z in/out
    // Triangle pointing UP
    simd::float3 triangleVertices[] = {
        {-0.5f, -0.5f, 0.0f}, // Bottom Left
        { 0.5f, -0.5f, 0.0f}, // Bottom Right
        { 0.0f,  0.5f, 0.0f}  // Top Center
    };
    
    std::cout << "Creating triangle buffer with " << sizeof(triangleVertices) << " bytes." << std::endl;

    triangleVertexBuffer = Device->newBuffer(&triangleVertices,
                                                  sizeof(triangleVertices),
                                                  MTL::ResourceStorageModeShared);
}

void Metal::createDefaultLibrary() {
    metalDefaultLibrary = Device->newDefaultLibrary();
    if(metalDefaultLibrary) return;

    std::cout << "Default library not found, attempting to load from source..." << std::endl;

    // Search for triangle.metal
    std::filesystem::path exePath;
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string pathStr(size, '\0');
    _NSGetExecutablePath(pathStr.data(), &size);
    // Remove null terminator if present
    if (!pathStr.empty() && pathStr.back() == '\0') pathStr.pop_back();
    exePath = std::filesystem::path(pathStr).parent_path();

    std::vector<std::filesystem::path> searchPaths;
    std::filesystem::path current = exePath;
    for(int i=0; i<6; ++i) {
        searchPaths.push_back(current);
        if(current.has_parent_path()) current = current.parent_path();
        else break;
    }

    std::filesystem::path sourcePath;
    for(const auto& p : searchPaths) {
        auto potential = p / "Shaders" / "triangle.metal";
        if(std::filesystem::exists(potential)) {
            sourcePath = potential;
            break;
        }
        potential = p / "Resources" / "triangle.metal";
        if(std::filesystem::exists(potential)) {
             sourcePath = potential;
             break;
        }
        potential = p / "Contents" / "Resources" / "triangle.metal";
        if(std::filesystem::exists(potential)) {
             sourcePath = potential;
             break;
        }
    }

    if(sourcePath.empty()) {
        std::cerr << "Failed to find triangle.metal source." << std::endl;
        std::exit(-1);
    }

    std::ifstream file(sourcePath);
    if(!file.is_open()) {
        std::cerr << "Failed to open " << sourcePath << std::endl;
        std::exit(-1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    NS::Error* error = nullptr;
    metalDefaultLibrary = Device->newLibrary(NS::String::string(source.c_str(), NS::UTF8StringEncoding), nullptr, &error);

    if(!metalDefaultLibrary) {
        std::cerr << "Failed to compile library from source: " << error->localizedDescription()->utf8String() << std::endl;
        std::exit(-1);
    }
    
    std::cout << "Loaded library from source: " << sourcePath << std::endl;
}

void Metal::createCommandQueue() {
    metalCommandQueue = Device->newCommandQueue();
}

void Metal::createRenderPipeline() {
    MTL::Function* vertexShader = metalDefaultLibrary->newFunction(NS::String::string("vertex_main", NS::ASCIIStringEncoding));
    assert(vertexShader);
    MTL::Function* fragmentShader = metalDefaultLibrary->newFunction(NS::String::string("fragment_main", NS::ASCIIStringEncoding));
    assert(fragmentShader);

    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline", NS::ASCIIStringEncoding));
    renderPipelineDescriptor->setVertexFunction(vertexShader);
    renderPipelineDescriptor->setFragmentFunction(fragmentShader);
    assert(renderPipelineDescriptor);
    MTL::PixelFormat pixelFormat = (MTL::PixelFormat)metalLayer.pixelFormat;
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);

    NS::Error* error;
    metalRenderPSO = Device->newRenderPipelineState(renderPipelineDescriptor, &error);

    renderPipelineDescriptor->release();
}

void Metal::draw() {
    sendRenderCommand();
}

void Metal::sendRenderCommand() {
    id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
    metalDrawable = (__bridge CA::MetalDrawable*)drawable;

    metalCommandBuffer = metalCommandQueue->commandBuffer();

    MTL::RenderPassDescriptor* renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
    MTL::RenderPassColorAttachmentDescriptor* cd = renderPassDescriptor->colorAttachments()->object(0);
    
    id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)metalDrawable;
    cd->setTexture((__bridge MTL::Texture*)currentDrawable.texture);
    
    cd->setLoadAction(MTL::LoadActionClear);
    cd->setClearColor(MTL::ClearColor(41.0f/255.0f, 42.0f/255.0f, 48.0f/255.0f, 1.0));
    cd->setStoreAction(MTL::StoreActionStore);

    MTL::RenderCommandEncoder* renderCommandEncoder = metalCommandBuffer->renderCommandEncoder(renderPassDescriptor);
    encodeRenderCommand(renderCommandEncoder);

    if (ImGuiRenderCallback) {
        ImGuiRenderCallback(renderCommandEncoder);
    }

    renderCommandEncoder->endEncoding();

    metalCommandBuffer->presentDrawable((__bridge MTL::Drawable*)currentDrawable);
    metalCommandBuffer->commit();
    metalCommandBuffer->waitUntilCompleted();

    renderPassDescriptor->release();
}

void Metal::encodeRenderCommand(MTL::RenderCommandEncoder* renderCommandEncoder) {
    renderCommandEncoder->setRenderPipelineState(metalRenderPSO);
    renderCommandEncoder->setVertexBuffer(triangleVertexBuffer, 0, 0);
    MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
    NS::UInteger vertexStart = 0;
    NS::UInteger vertexCount = 3;
    renderCommandEncoder->drawPrimitives(typeTriangle, vertexStart, vertexCount);
}

void Metal::Cleanup() {
    if (Device) {
        Device->release();
        Device = nullptr;
    }
}

#endif
