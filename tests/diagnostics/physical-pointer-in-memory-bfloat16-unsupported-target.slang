//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target hlsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target hlsl -entry pointerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM): -target hlsl -entry parameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute -O0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry pointerMain -stage compute

// WGSL and HLSL have no pointers to memory addresses, and WGSL has no bfloat16 type. A pointer read
// from memory (here, from a constant buffer) and a `BFloat16` value must be diagnosed where they
// are read, not emitted as invalid code (#13632). A pointer passed on to a helper function is not
// diagnosed again inside it, whether the helper reads it directly, carries it through a loop or
// stores it in a local array.

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

float4 sumNodes(Node* node, uint count)
{
    float4 sum = 0;
    for (uint i = 0; i < count; i++)
    {
        sum += node->value;
        node = node + 1;
    }
    return sum;
}

float4 pickNode(Node* a, Node* b, uint index)
{
    Node* candidates[2] = { a, b };
    return candidates[index & 1]->value;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void pointerMain(uint3 tid: SV_DispatchThreadID)
{
    Node* nodes = pointers.nodes;
//PTR:    ^^^^^ pointers to memory addresses are not supported on this target
//PTR:    ^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
    outputBuffer[tid.x] = readNode(nodes, tid.x) + sumNodes(nodes, tid.x) + pickNode(nodes, nodes, tid.x);
}

// SPIR-V has physical storage buffer pointers, so nothing is diagnosed there.
// SPV: OpTypePointer PhysicalStorageBuffer

// HLSL keeps a pointer-typed global or entry-point parameter as a parameter; it is diagnosed at
// its declaration.
uniform Node* globalNode;
//PARAM:      ^^^^^^^^^^ pointers to memory addresses are not supported on this target
//PARAM:      ^^^^^^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(64, 1, 1)]
void parameterMain(uint3 tid: SV_DispatchThreadID, uniform Node* entryPointNode)
//PARAM:                                                         ^^^^^^^^^^^^^^ pointers to memory addresses are not supported on this target
//PARAM:                                                         ^^^^^^^^^^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
{
    outputBuffer[tid.x] = globalNode->value + entryPointNode->value;
}

struct BFloat16Pair
{
    BFloat16 a;
    BFloat16 b;
};

StructuredBuffer<BFloat16> scalars;
StructuredBuffer<BFloat16Pair> pairs;
RWStructuredBuffer<BFloat16Pair> outPairs;

[shader("compute")]
[numthreads(64, 1, 1)]
void bfloat16Main(uint3 tid: SV_DispatchThreadID)
{
    const float x = float(scalars[tid.x]);
//BF16:                          ^ 'BFloat16' is not supported on this target
//BF16:                          ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
    // A struct that contains a `BFloat16` is diagnosed where it is read, not at each field.
    const BFloat16Pair pair = pairs[tid.x];
//BF16:                            ^ 'BFloat16' is not supported on this target
//BF16:                            ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
    outputBuffer[tid.x] = float4(x, float(pair.a), float(pair.b), 0);
    // Also when it is only copied.
    outPairs[tid.x] = pairs[tid.x + 1];
//BF16:                    ^ 'BFloat16' is not supported on this target
//BF16:                    ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
}
