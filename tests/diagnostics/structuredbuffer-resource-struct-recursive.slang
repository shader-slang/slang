//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

struct RecursiveFoo
{
    float value;
    //CHECK-DAG: ([[# @LINE+1]]): error 38205
    StructuredBuffer<RecursiveFoo> children;
}

StructuredBuffer<RecursiveFoo> recursiveRoot;
RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    RecursiveFoo foo = recursiveRoot[i];
    float result = foo.value + foo.children[0].value + foo.children[0].children[0].value;
    output[i] = result;
}

