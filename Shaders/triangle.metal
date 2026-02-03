#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float3 color;
};

vertex VertexOut vertex_main(uint vertexID [[vertex_id]],
                             constant float3* vertices [[buffer(0)]]) {
    VertexOut out;
    float3 pos = vertices[vertexID];
    out.position = float4(pos, 1.0);

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    out.color = colors[vertexID % 3];
    
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return float4(in.color, 1.0);
}
