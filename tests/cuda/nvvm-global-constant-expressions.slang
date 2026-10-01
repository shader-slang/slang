// Pure global expressions must be localized without changing their values or dependencies.
// NVRTC rejects the emitted dynamic __device__ initializer for kAdjusted.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Keep the motivating Float3 Add: this shape remains module-owned before NVVM legalization.
static const float3 kA = float3(0.25, 0.25, 1.0);
static const float3 kB = float3(1.0, 2.0, 3.0);
static const float3 kSum = kA + kB;
static const float3 kScaled = (kSum - kB) * float3(4.0, 8.0, 2.0);
static const int3 kIntegers = int3(
    int(kSum.x * 4.0),
    int(kSum.y * 4.0),
    int(kSum.z * 2.0));
static const int3 kAdjusted = kIntegers - int3(7, 5, 11);
static const float3 kVectors[2] = {kSum, kScaled};

struct ExpressionRecord
{
    float3 value;
    int3 integers;
};

static const float3 kTail = kVectors[0] + kVectors[1];

//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
float3 useFloatGlobals(float3 delta)
{
    ExpressionRecord first = {kVectors[0], kIntegers};
    return first.value + kTail + delta;
}

[noinline]
int3 useIntegerGlobals()
{
    ExpressionRecord first = {kVectors[0], kIntegers};
    ExpressionRecord second = {kVectors[1], kAdjusted};
    return first.integers + second.integers;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Synthesized record constructors execute in blocks; module calls remain outside this policy.
    ExpressionRecord first = {kVectors[0], kIntegers};
    ExpressionRecord second = {kVectors[1], kAdjusted};
    float3 helperValue = useFloatGlobals(float3(0.5, -0.5, 1.0));
    int3 helperIntegers = useIntegerGlobals();
    float3 repeated = first.value + second.value;
    float3 converted = float3(kAdjusted);
    [unroll]
    for (uint lane = 0; lane < 3; ++lane)
    {
        outputBuffer[1 + lane] = asuint(kSum[lane]);
        outputBuffer[4 + lane] = asuint(kScaled[lane]);
        outputBuffer[7 + lane] = asuint(first.integers[lane]);
        outputBuffer[10 + lane] = asuint(second.integers[lane]);
        outputBuffer[13 + lane] = asuint(kTail[lane]);
        outputBuffer[16 + lane] = asuint(helperValue[lane]);
        outputBuffer[19 + lane] = asuint(repeated[lane]);
        outputBuffer[22 + lane] = asuint(helperIntegers[lane]);
        outputBuffer[25 + lane] = asuint(converted[lane]);
    }
    outputBuffer[28] = 4096;
}

// Exact independent values: sum=(1.25,2.25,4), scaled=(1,2,2), integers=(5,9,8),
// adjusted=(-2,4,-3), tail=(2.25,4.25,6), helper=(4,6,11), integer helper=(3,13,5).
// Both boundary words retain the initializer; all interior words are checked.
//CHECK: {{^}}type: uint32_t{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
//CHECK-NEXT: {{^}}1067450368{{$}}
//CHECK-NEXT: {{^}}1074790400{{$}}
//CHECK-NEXT: {{^}}1082130432{{$}}
//CHECK-NEXT: {{^}}1065353216{{$}}
//CHECK-NEXT: {{^}}1073741824{{$}}
//CHECK-NEXT: {{^}}1073741824{{$}}
//CHECK-NEXT: {{^}}5{{$}}
//CHECK-NEXT: {{^}}9{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}4294967294{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}4294967293{{$}}
//CHECK-NEXT: {{^}}1074790400{{$}}
//CHECK-NEXT: {{^}}1082654720{{$}}
//CHECK-NEXT: {{^}}1086324736{{$}}
//CHECK-NEXT: {{^}}1082130432{{$}}
//CHECK-NEXT: {{^}}1086324736{{$}}
//CHECK-NEXT: {{^}}1093664768{{$}}
//CHECK-NEXT: {{^}}1074790400{{$}}
//CHECK-NEXT: {{^}}1082654720{{$}}
//CHECK-NEXT: {{^}}1086324736{{$}}
//CHECK-NEXT: {{^}}3{{$}}
//CHECK-NEXT: {{^}}13{{$}}
//CHECK-NEXT: {{^}}5{{$}}
//CHECK-NEXT: {{^}}3221225472{{$}}
//CHECK-NEXT: {{^}}1082130432{{$}}
//CHECK-NEXT: {{^}}3225419776{{$}}
//CHECK-NEXT: {{^}}4096{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
