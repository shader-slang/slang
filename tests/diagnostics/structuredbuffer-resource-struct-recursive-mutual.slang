//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

struct MutualB
{
    //CHECK-DAG: ([[# @LINE+1]]): error 38205
    StructuredBuffer<MutualA> aBuffer;
}

struct MutualA
{
    //CHECK-DAG: ([[# @LINE+1]]): error 38205
    StructuredBuffer<MutualB> bBuffer;
}

StructuredBuffer<MutualA> mutualRoot;
RWStructuredBuffer<float> output;

// External function to force consumption of recursive types
float consumeMutual(MutualA a, MutualB b);

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    float result = 0;
    // Force usage of mutual recursion
    MutualA a = mutualRoot[i];
    MutualB b = a.bBuffer[0];
    result += consumeMutual(a, b);

    output[i] = result;
}

