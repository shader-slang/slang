//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target hlsl -entry pointerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target wgsl -entry pointerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=PTR): -target hlsl -entry pointerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM): -target hlsl -entry parameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM): -target hlsl -entry parameterMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMW): -target wgsl -entry parameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMW): -target wgsl -entry parameterMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16): -target wgsl -entry bfloat16Main -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16C): -target wgsl -entry bfloat16ConstantMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16C): -target wgsl -entry bfloat16ConstantMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16M): -target wgsl -entry bfloat16MoreMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BF16M): -target wgsl -entry bfloat16MoreMain -stage compute -O0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry pointerMain -stage compute

// WGSL and HLSL have no pointers to memory addresses, and WGSL has no bfloat16 type. A pointer read
// from memory (here, from a constant buffer) and a `BFloat16` value must be diagnosed where they
// are read, not emitted as invalid code (#13632). A pointer passed on to a helper function is not
// diagnosed again inside it, whether the helper reads it directly, carries it through a loop,
// stores it in a local array or takes it as an `inout` parameter.

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

float4 advanceNode(inout Node* node)
{
    const float4 value = node->value;
    node = node + 1;
    return value;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void pointerMain(uint3 tid: SV_DispatchThreadID)
{
    Node* nodes = pointers.nodes;
//PTR:    ^^^^^ pointers to memory addresses are not supported on this target
//PTR:    ^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
    outputBuffer[tid.x] = readNode(nodes, tid.x) + sumNodes(nodes, tid.x) + pickNode(nodes, nodes, tid.x);
    outputBuffer[tid.x + 1] = advanceNode(nodes) + advanceNode(nodes);
}

// SPIR-V has physical storage buffer pointers, so nothing is diagnosed there.
// SPV: OpTypePointer PhysicalStorageBuffer

// HLSL keeps a pointer-typed global or entry-point parameter as a parameter; it is diagnosed at
// its declaration. WGSL passes both in a uniform buffer, so each is diagnosed where it is read.
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
//PARAMW:                             ^^^^^ pointers to memory addresses are not supported on this target
//PARAMW:                             ^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
//PARAMW:                                                     ^^^^^ pointers to memory addresses are not supported on this target
//PARAMW:                                                     ^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
}

struct BFloat16Pair
{
    BFloat16 a;
    BFloat16 b;
};

StructuredBuffer<BFloat16> scalars;
StructuredBuffer<BFloat16Pair> pairs;
StructuredBuffer<vector<BFloat16, 4>> vectors;
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
    // A vector of `BFloat16`.
    outputBuffer[tid.x + 1] = float4(vectors[tid.x]);
//BF16:                                     ^ 'BFloat16' is not supported on this target
//BF16:                                     ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
}

RWStructuredBuffer<BFloat16> outScalars;

[shader("compute")]
[numthreads(64, 1, 1)]
void bfloat16ConstantMain(uint3 tid: SV_DispatchThreadID)
{
    // A `BFloat16` constant is folded and hoisted to module scope; it is diagnosed where it is used.
    outScalars[tid.x] = BFloat16(1.5);
//BF16C:              ^ 'BFloat16' is not supported on this target
//BF16C:              ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
}

StructuredBuffer<matrix<BFloat16, 2, 2>> matrices;
StructuredBuffer<float> floats;

[shader("compute")]
[numthreads(64, 1, 1)]
void bfloat16MoreMain(uint3 tid: SV_DispatchThreadID, uniform BFloat16 scale)
{
    // A matrix of `BFloat16`. Its storage is loaded by an instruction without a location, so it
    // is diagnosed at the nearest use that has one.
    const matrix<BFloat16, 2, 2> m = matrices[tid.x];
    outputBuffer[tid.x] = float4(float(m[0][0]), float(m[1][1]), 0, 0);
//BF16M:                                              ^ 'BFloat16' is not supported on this target
//BF16M:                                              ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
    // A conversion to `BFloat16` at run time.
    outScalars[tid.x] = BFloat16(floats[tid.x]);
//BF16M:                        ^ 'BFloat16' is not supported on this target
//BF16M:                        ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
    // A uniform parameter, which WGSL passes in a uniform buffer, so it is diagnosed where it is
    // read.
    outputBuffer[tid.x + 1] = float4(float(scale), 0, 0, 0);
//BF16M:                                  ^ 'BFloat16' is not supported on this target
//BF16M:                                  ^ this uses a 'BFloat16' value, and the current code generation target has no bfloat16 type
}
