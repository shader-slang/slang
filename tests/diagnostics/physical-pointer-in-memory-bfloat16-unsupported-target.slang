//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target hlsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute -O0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry pointerMain -stage compute

// WGSL and HLSL have no pointers to memory addresses, and WGSL has no bfloat16 type. A pointer read
// from memory (here, from a constant buffer) and a `BFloat16` value must be diagnosed where they
// are read, not emitted as invalid code (#13632). A pointer passed on to a helper function is not
// diagnosed again inside it.

struct Node
{
    float4 value;
};

struct Pointers
{
    Node* nodes;
    uint count;
};

ConstantBuffer<Pointers> pointers;
RWStructuredBuffer<float4> outputBuffer;

float4 readNode(Node* nodes, uint index)
{
    return nodes[index].value;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void pointerMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = readNode(pointers.nodes, tid.x);
//PTR:                            ^ pointers to memory addresses are not supported on this target
//PTR:                            ^ this creates a pointer from an address, reads one from memory or uses 'nullptr', but the current code generation target has no pointers to memory addresses
}

// SPIR-V has physical storage buffer pointers, so nothing is diagnosed there.
// SPV: OpTypePointer PhysicalStorageBuffer

struct Half2
{
    BFloat16 a;
    BFloat16 b;
};

StructuredBuffer<BFloat16> scalars;
StructuredBuffer<Half2> pairs;

[shader("compute")]
[numthreads(64, 1, 1)]
void bfloat16Main(uint3 tid: SV_DispatchThreadID)
{
    const float x = float(scalars[tid.x]);
//BF16:                          ^ 'BFloat16' is not supported on this target
//BF16:                          ^ this creates a 'BFloat16' value, but the current code generation target has no bfloat16 type
    // Each field read from a struct is a new value.
    const Half2 pair = pairs[tid.x];
    outputBuffer[tid.x] = float4(x, float(pair.a), float(pair.b), 0);
//BF16:                                  ^ 'BFloat16' is not supported on this target
//BF16:                                  ^ this creates a 'BFloat16' value, but the current code generation target has no bfloat16 type
//BF16:                                                 ^ 'BFloat16' is not supported on this target
//BF16:                                                 ^ this creates a 'BFloat16' value, but the current code generation target has no bfloat16 type
}
