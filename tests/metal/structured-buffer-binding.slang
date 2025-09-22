//TEST:SIMPLE(filecheck=CHECK): -stage vertex -entry vertexMain -target metal
//TEST(smoke,render):COMPARE_RENDER_COMPUTE(filecheck-buffer=BUF):-metal -shaderobj -output-using-type  
  
// Test that StructuredBuffer with explicit register() and [[vk::binding()]] annotations
// generates proper [[buffer(N)]] attributes in Metal output (fixes issue #7669)

static const float2 rect_to_triangle_mapping[] = {
    float2(-0.5, 0.5),
    float2(0.5, 0.5),
    float2(-0.5, -0.5),
    float2(-0.5, -0.5),
    float2(0.5, 0.5),
    float2(0.5, -0.5),
};

struct Rect
{
    float2 center;
    float2 size;
};

struct VRectOut
{
    float4 pos : SV_Position;
    nointerpolation uint32_t rect_id;
};

// Minimal cbuffer for render test framework - should be collected into global scope  
cbuffer Uniforms
{
    float4x4 modelViewProjection;
}
  
// CHECK: {{\[\[}}vertex{{\]\]}} vertexMain_Result_{{.*}} vertexMain(Rect_{{.*}} device* rects_{{.*}} {{\[\[}}buffer(1){{\]\]}}, float device* outputBuffer_{{.*}} {{\[\[}}buffer(2){{\]\]}}, uint vertex_id_{{.*}} {{\[\[}}vertex_id{{\]\]}})  
[shader("vertex")]  
VRectOut vertexMain(  
//TEST_INPUT: set rects = ubuffer(data=[1.0 2.0 10.0 5.0  3.0 4.0 8.0 6.0], stride=16);  
    [[vk::binding(0, 0)]] const StructuredBuffer<Rect> rects: register(u0, space0),  
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer  
    RWStructuredBuffer<float> outputBuffer,  
    uint32_t vertex_id: SV_VertexID)  
{  
    VRectOut output;  
    uint32_t rect_id = vertex_id / 6;  
    float area = 0.f;
    Rect r = rects[rect_id];  
    output.pos.xy = r.center + rect_to_triangle_mapping[vertex_id % 6] * r.size;  
    output.pos.w = 1.0;  
    output.rect_id = rect_id;  
      
    // Calculate and store area in output buffer  
    if (vertex_id == 0) {  
        // Calculate area for rectangle 0: rects[0]  
        Rect r0 = rects[0];  
        area = r0.size.x * r0.size.y; // 10.0 * 5.0 = 50.0  
    }  
    else if (vertex_id == 1) {  
        // Calculate area for rectangle 1: rects[1]    
        Rect r1 = rects[1];  
        area = r1.size.x * r1.size.y; // 8.0 * 6.0 = 48.0  
    }
    outputBuffer[vertex_id] = area;
      
    return output;
}  
  
[shader("fragment")]  
float4 fragmentMain(VRectOut input) : SV_Target  
{  
    // Simple fragment shader for graphics pipeline completion  
    return float4(1.0, 1.0, 1.0, 1.0);  
}  
  
// Verify the computed areas: rect0 = 10*5=50, rect1 = 8*6=48  
//BUF: type: float  
//BUF-NEXT: 50.000000  
//BUF-NEXT: 48.000000  
//BUF-NEXT: 0.000000
//BUF-NEXT: 0.000000

