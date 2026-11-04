//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=FRAG): -target metal -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=FRAGLIB): -target metallib -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=BOTH): -target metal -entry computeMain -stage compute -entry fragMain -stage fragment

// Metal accepts `[[raster_order_group]]` on the parameters of a fragment function only; on a
// kernel parameter it is an error ("invalid 'raster_order_group' attribute for input
// declaration in a kernel function"). A rasterizer-ordered resource used by a compute kernel
// is emitted as the plain read-write resource.

RasterizerOrderedStructuredBuffer<int> rov;
RasterizerOrderedTexture2D<float> rovTex;
RWStructuredBuffer<int> outputBuffer;

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(
// METAL-NOT: raster_order_group
// METAL: return;
// METALLIB: @computeMain
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    rov[tid.x] = rov[tid.x] + 1;
    rovTex[tid.xy] = rovTex[tid.xy] + 1.0;
    outputBuffer[tid.x] = rov[tid.x] + int(rovTex[tid.xy]);
}

// FRAG: {{\[\[}}fragment{{\]\]}} {{.*}} fragMain({{.*}}raster_order_group(0){{.*}}raster_order_group(0)

// In one module with both entry points, only the fragment parameters keep the attribute.
// BOTH: {{\[\[}}kernel{{\]\]}} void computeMain(
// BOTH-NOT: raster_order_group
// BOTH: {{\[\[}}fragment{{\]\]}} {{.*}} fragMain({{.*}}raster_order_group(0){{.*}}raster_order_group(0)
// FRAGLIB: @fragMain
[shader("fragment")]
float4 fragMain(float4 pos: SV_Position) : SV_Target
{
    uint2 p = uint2(pos.xy);
    rov[p.x] = rov[p.x] + 1;
    rovTex[p] = rovTex[p] + 1.0;
    return float4(float(rov[p.x]), rovTex[p], 0, 1);
}
