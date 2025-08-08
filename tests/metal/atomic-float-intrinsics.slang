//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute -DMETAL
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute -DMETAL
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-mtl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=input
RWStructuredBuffer<float> input;

//TEST_INPUT: ubuffer(data=[0.0 0.0], stride=4):out,name=output
RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint dst = tid.x % 2;  // Multiple threads access same output indices
    
    //MTL: atomic_fetch_add_explicit
    //LIB: @air.atomic.global.add.f32
    InterlockedAdd(output[dst], input[tid.x]);
    
    float originalValue;
    //MTL: atomic_fetch_add_explicit
    //LIB: @air.atomic.global.add.f32
    InterlockedAdd(output[dst], input[tid.x] * 0.5f, originalValue);
    
    //MTL: atomic_fetch_add_explicit
    //LIB: @air.atomic.global.add.f32
    InterlockedAdd(output[dst], -input[tid.x] * 0.25f);
    
    float originalValue2;
    //MTL: atomic_fetch_add_explicit
    //LIB: @air.atomic.global.add.f32
    InterlockedAdd(output[dst], -input[tid.x] * 0.1f, originalValue2);
}

// Expected values with dst = tid.x % 2:
// Threads 0,2 -> dst=0: (1.0 + 3.0) + (0.5 + 1.5) - (0.25 + 0.75) - (0.1 + 0.3) = 4.6
// Threads 1,3 -> dst=1: (2.0 + 4.0) + (1.0 + 2.0) - (0.5 + 1.0) - (0.2 + 0.4) = 6.9
// CHK: 4.6
// CHK: 6.9