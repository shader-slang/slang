// A `unorm`/`snorm` modifier on a non-texture type that needs a layout — a
// structured-buffer element, or a field of a module-scope uniform struct — must
// lay out as its base type and read back at full precision (the modifier only
// affects texture image formats, not buffer/uniform storage).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o unorm-snorm-layout-12535.spv

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<unorm float> outputBuffer;

// Seeded with a value that is not the all-zero bit pattern: a broken snorm read
// also yields 0, so checking against 0 would pass either way.
//TEST_INPUT:ubuffer(data=[-0.25], stride=4):name=snormBuffer
StructuredBuffer<snorm float> snormBuffer;

// `v` (a `unorm float4`) is first so its offset matches under natural and
// constant-buffer layout, and its full 16-byte read-back would catch a
// regression to RGBA8-packed storage.
struct Norms
{
    unorm float4 v;
    unorm float u;
    snorm float s;
}
//TEST_INPUT:uniform(data=[1.0 2.0 3.0 4.0 0.5 -0.5]):name=n
uniform Norms n;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = n.v.x;          // 1.0
    outputBuffer[1] = n.v.w;          // 4.0
    outputBuffer[2] = n.u;            // 0.5
    outputBuffer[3] = n.s;            // -0.5
    outputBuffer[4] = snormBuffer[0]; // -0.25
    // CHECK: 1.0
    // CHECK: 4.0
    // CHECK: 0.5
    // CHECK: -0.5
    // CHECK: -0.25
}
