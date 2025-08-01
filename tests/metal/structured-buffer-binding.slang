//TEST:SIMPLE(filecheck=CHECK): -stage vertex -entry rects_vert -target metal

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

// CHECK: {{\[\[}}vertex{{\]\]}} rects_vert_Result_{{.*}} rects_vert(Rect_{{.*}} device* rects_{{.*}} {{\[\[}}buffer(0){{\]\]}}, uint vertex_id_{{.*}} {{\[\[}}vertex_id{{\]\]}})
[shader("vertex")]
VRectOut rects_vert(
    [[vk::binding(0, 0)]] const StructuredBuffer<Rect> rects: register(u0, space0),
    uint32_t vertex_id: SV_VertexID)
{
    VRectOut output;
    uint32_t rect_id = vertex_id / 6;
    Rect r = rects[rect_id];
    output.pos.xy = r.center + rect_to_triangle_mapping[vertex_id % 6] * r.size;
    output.pos.w = 1.0;
    output.rect_id = rect_id;
    return output;
}

