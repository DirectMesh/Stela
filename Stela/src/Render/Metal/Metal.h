#pragma once

#if defined(__APPLE__)

#include <simd/simd.h>
#include <functional>

namespace MTL { class Device; class Library; class CommandQueue; class CommandBuffer; class RenderPipelineState; class Buffer; class RenderCommandEncoder; }
namespace CA { class MetalDrawable; }

#ifdef __OBJC__
@class CAMetalLayer;
@class NSWindow;
#else
class CAMetalLayer;
class NSWindow;
#endif

struct SDL_Window;

class Metal {
public:
    
    MTL::Device* Device;
    NSWindow* metalWindow;
    CAMetalLayer* metalLayer;
    CA::MetalDrawable* metalDrawable;

    MTL::Library* metalDefaultLibrary;
    MTL::CommandQueue* metalCommandQueue;
    MTL::CommandBuffer* metalCommandBuffer;
    MTL::RenderPipelineState* metalRenderPSO;
    MTL::Buffer* triangleVertexBuffer;
    
    std::function<void(MTL::RenderCommandEncoder*)> ImGuiRenderCallback;

    void Init(SDL_Window* window);
    void CreateDevice();
    void createTriangle();
    void createDefaultLibrary();
    void createCommandQueue();
    void createRenderPipeline();

    void encodeRenderCommand(MTL::RenderCommandEncoder* renderEncoder);
    void sendRenderCommand();
    void draw();
    void Cleanup();
};

#endif
