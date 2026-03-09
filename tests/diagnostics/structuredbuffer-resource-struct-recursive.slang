//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct RecursiveFoo
{
    float value;
    StructuredBuffer<RecursiveFoo> children;
//CHECK:                           ^^^^^^^^ recursive type in structured buffer
//CHECK:                           ^^^^^^^^ structured buffer element type 'RecursiveFoo' contains recursive type references
}

StructuredBuffer<RecursiveFoo> recursiveRoot;
//CHECK:                       ^^^^^^^^^^^^^ recursive type in structured buffer
//CHECK:                       ^^^^^^^^^^^^^ structured buffer element type 'RecursiveFoo' contains recursive type references
RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    RecursiveFoo foo = recursiveRoot[i];
    float result = foo.value + foo.children[0].value + foo.children[0].children[0].value;
    output[i] = result;
}

