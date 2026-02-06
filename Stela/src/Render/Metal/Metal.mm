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
    createOffscreenResources();
    createCommandQueue();
    createRenderPipeline();
}

void Metal::createOffscreenResources() {
    MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::alloc()->init();
    textureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    textureDescriptor->setWidth((NS::UInteger)metalLayer.drawableSize.width);
    textureDescriptor->setHeight((NS::UInteger)metalLayer.drawableSize.height);
    textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    textureDescriptor->setStorageMode(MTL::StorageModePrivate);

    OffscreenTexture = Device->newTexture(textureDescriptor);
    textureDescriptor->release();
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

    if (ImGuiRenderCallback) {
        // 1. Offscreen Pass
        MTL::RenderPassDescriptor* offscreenPass = MTL::RenderPassDescriptor::alloc()->init();
        MTL::RenderPassColorAttachmentDescriptor* ca = offscreenPass->colorAttachments()->object(0);
        ca->setTexture(OffscreenTexture);
        ca->setLoadAction(MTL::LoadActionClear);
        ca->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));
        ca->setStoreAction(MTL::StoreActionStore);

        MTL::RenderCommandEncoder* offscreenEncoder = metalCommandBuffer->renderCommandEncoder(offscreenPass);
        encodeRenderCommand(offscreenEncoder);
        offscreenEncoder->endEncoding();
        offscreenPass->release();

        // 2. Drawable Pass (UI)
        MTL::RenderPassDescriptor* drawablePass = MTL::RenderPassDescriptor::alloc()->init();
        MTL::RenderPassColorAttachmentDescriptor* da = drawablePass->colorAttachments()->object(0);
        
        id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)metalDrawable;
        da->setTexture((__bridge MTL::Texture*)currentDrawable.texture);
        
        da->setLoadAction(MTL::LoadActionClear);
        da->setClearColor(MTL::ClearColor(41.0f/255.0f, 42.0f/255.0f, 48.0f/255.0f, 1.0));
        da->setStoreAction(MTL::StoreActionStore);

        MTL::RenderCommandEncoder* drawableEncoder = metalCommandBuffer->renderCommandEncoder(drawablePass);

        ImGuiRenderCallback(drawableEncoder);
        
        drawableEncoder->endEncoding();
        drawablePass->release();
    } else {
        // Runtime Mode: Render directly to Drawable
        MTL::RenderPassDescriptor* drawablePass = MTL::RenderPassDescriptor::alloc()->init();
        MTL::RenderPassColorAttachmentDescriptor* da = drawablePass->colorAttachments()->object(0);
        
        id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)metalDrawable;
        da->setTexture((__bridge MTL::Texture*)currentDrawable.texture);
        
        da->setLoadAction(MTL::LoadActionClear);
        da->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));
        da->setStoreAction(MTL::StoreActionStore);

        MTL::RenderCommandEncoder* encoder = metalCommandBuffer->renderCommandEncoder(drawablePass);
        encodeRenderCommand(encoder);
        encoder->endEncoding();
        drawablePass->release();
    }

    metalCommandBuffer->presentDrawable((__bridge MTL::Drawable*)drawable);
    metalCommandBuffer->commit();
    metalCommandBuffer->waitUntilCompleted();
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
