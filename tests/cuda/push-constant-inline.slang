// push-constant-inline.slang

// Test that [[vk::push_constant]] parameters are inlined by value
// in GlobalParams on CUDA/CPP targets, rather than being lowered
// to pointer indirection through ConstantBuffer<T>.

//TEST:SIMPLE(filecheck=CUDA_IMPLICIT):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP_IMPLICIT):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA_EXPLICIT):-target cuda -entry computeMainExplicit -stage compute
//TEST:SIMPLE(filecheck=CPP_EXPLICIT):-target cpp -entry computeMainExplicit -stage compute

struct PushData
{
    float scale;
    float offset;
};

[[vk::push_constant]]
PushData pushConstants;

RWStructuredBuffer<float> outputBuffer;
[[vk::push_constant]]
ConstantBuffer<PushData> explicitPushConstants;
RWStructuredBuffer<float> explicitOutputBuffer;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = pushConstants.scale + pushConstants.offset;
}

[numthreads(1,1,1)]
void computeMainExplicit(uint3 tid : SV_DispatchThreadID)
{
    explicitOutputBuffer[tid.x] =
        explicitPushConstants.scale + explicitPushConstants.offset;
}

// Verify push constant struct is inlined by value (not a pointer) in GlobalParams.
// CUDA_IMPLICIT: struct GlobalParams
// CUDA_IMPLICIT-NOT: PushData{{.*}}*
// CUDA_IMPLICIT: PushData{{.*}}pushConstants

// CPP_IMPLICIT: struct GlobalParams
// CPP_IMPLICIT-NOT: PushData{{.*}}*
// CPP_IMPLICIT: PushData{{.*}}pushConstants

// CUDA_EXPLICIT: struct GlobalParams
// CUDA_EXPLICIT-NOT: PushData{{.*}}*
// CUDA_EXPLICIT: PushData{{.*}}explicitPushConstants

// CPP_EXPLICIT: struct GlobalParams
// CPP_EXPLICIT-NOT: PushData{{.*}}*
// CPP_EXPLICIT: PushData{{.*}}explicitPushConstants
