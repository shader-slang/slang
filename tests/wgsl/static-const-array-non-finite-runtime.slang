//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// Runtime companion to static-const-array-non-finite.slang (issue #13658). The NaN and infinity
// entries of a runtime-indexed `static const` table must read back as NaN and infinity. On WGSL the
// table is built inside the function from the `_slang_getNan` / `_slang_getInfinity` helpers; the
// NVIDIA Vulkan driver crashed creating this pipeline while those helpers divided by zero.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const float kTable[3] = { 0.0 / 0.0, 1.0 / 0.0, 2.0 };
static const float4 kVecs[2] = { float4(-1.0 / 0.0, 0.0, 0.0, 0.0), float4(1.0, 2.0, 3.0, 4.0) };

// 1 = NaN, 2 = +infinity, 3 = -infinity, 10 + v for a finite v.
uint classify(float v)
{
    if (isnan(v))
        return 1;
    if (isinf(v))
        return v > 0 ? 2 : 3;
    return uint(v) + 10;
}

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    float v = (i < 3) ? kTable[i] : (i < 5) ? kVecs[i - 3].x : kTable[i % 3];
    outputBuffer[i] = classify(v);
}

// CHECK: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}12{{$}}
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}12{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
