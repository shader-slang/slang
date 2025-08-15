//TEST:SIMPLE(filecheck=CHECK_MTL): -target metal -stage compute -entry computeMain
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -render-features argument-buffer-tier-2 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// Test that StructuredBuffer with explicit register() and [[vk::binding()]] annotations
// generates proper [[buffer(N)]] attributes in Metal output and works correctly at runtime

struct Rect
{
    float2 center;
    float2 size;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK_MTL: {{\[\[}}kernel{{\]\]}} void computeMain({{.*}}Rect{{.*}} device* rects{{.*}} {{\[\[}}buffer(0){{\]\]}}{{.*}})
//TEST_INPUT: set rects = ubuffer(data=[1.0 2.0 10.0 5.0  3.0 4.0 8.0 6.0], stride=16);
[[vk::binding(0, 0)]] const StructuredBuffer<Rect> rects : register(t0, space0);

[shader("compute")]
[numthreads(4,1,1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint index = threadID.x;
    if (index < 2)
    {
        Rect r = rects[index];
        // Store area of the rectangle as output
        outputBuffer[index] = r.size.x * r.size.y;
    }
}

// Verify the computed areas: rect0 = 10*5=50, rect1 = 8*6=48
//CHECK: type: float
//CHECK-NEXT: 50.000000
//CHECK-NEXT: 48.000000
//CHECK-NEXT: 0.000000
//CHECK-NEXT: 0.000000