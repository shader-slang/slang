// sv-clip-distance-vector-render.slang

// Vector-typed SV_ClipDistanceN values (issue #13931) written by a vertex shader reach the
// fragment shader through the packed gl_ClipDistance array on Vulkan. CLIPPED gives the last
// element (SV_ClipDistance1.y, element 5) a negative distance, so the whole triangle is
// clipped away and the output buffer keeps its initial contents.

//TEST(compute, vulkan):COMPARE_RENDER_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST(compute, vulkan):COMPARE_RENDER_COMPUTE(filecheck-buffer=CLIPPED): -vk -output-using-type -xslang -DCLIP_LAST

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0], stride=4):out, name outputBuffer
RWStructuredBuffer<float> outputBuffer;

cbuffer Uniforms
{
    float4x4 modelViewProjection;
}

struct AssembledVertex
{
    float3 position;
    float3 color;
    float2 uv;
};

struct VertexStageInput
{
    AssembledVertex assembledVertex : A;
};

struct VertexStageOutput
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
    float2 clip1 : SV_ClipDistance1;
};

VertexStageOutput vertexMain(VertexStageInput input)
{
    VertexStageOutput output;
    output.position = mul(modelViewProjection, float4(input.assembledVertex.position, 1.0));
    output.clip0 = float4(1, 2, 3, 4);
#ifdef CLIP_LAST
    output.clip1 = float2(5, -1);
#else
    output.clip1 = float2(5, 6);
#endif
    return output;
}

float4 fragmentMain(float4 clip0: SV_ClipDistance0, float2 clip1: SV_ClipDistance1) : SV_Target
{
    outputBuffer[0] = clip0.x;
    outputBuffer[1] = clip0.y;
    outputBuffer[2] = clip0.z;
    outputBuffer[3] = clip0.w;
    outputBuffer[4] = clip1.x;
    outputBuffer[5] = clip1.y;
    return float4(1, 1, 1, 1);
}

// CHECK: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0

// CLIPPED: 0.0
// CLIPPED-NEXT: 0.0
// CLIPPED-NEXT: 0.0
// CLIPPED-NEXT: 0.0
// CLIPPED-NEXT: 0.0
// CLIPPED-NEXT: 0.0
