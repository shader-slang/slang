//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -profile lib_6_6
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0

// An exported function is kept without a caller in the module, so its parameters are set from
// outside, like an entry point's (#13632). A pointer parameter is diagnosed at its declaration,
// and a pointer read through an `inout` parameter where it is read. A helper the pointer is
// passed to is not diagnosed again.

public struct Node
{
    public float value;
};

float readValue(Node* node)
{
    return node->value;
}

export float readExported(Node* node, inout Node* cursor)
//CHECK:                        ^^^^ pointers to memory addresses are not supported on this target
//CHECK:                        ^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
{
    return readValue(node) + cursor->value;
//CHECK:                     ^^^^^^ pointers to memory addresses are not supported on this target
//CHECK:                     ^^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1.0;
}
