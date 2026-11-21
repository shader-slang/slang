//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target hlsl -entry unreadFieldMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target hlsl -entry unreadFieldMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target wgsl -entry unreadFieldMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target wgsl -entry unreadFieldMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target hlsl -entry unassignedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target hlsl -entry unassignedMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target wgsl -entry unassignedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target wgsl -entry unassignedMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=INIT): -target hlsl -entry initializerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INIT): -target hlsl -entry initializerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=INIT): -target wgsl -entry initializerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INIT): -target wgsl -entry initializerMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSET): -target hlsl -entry partlySetMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSET): -target hlsl -entry partlySetMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSET): -target wgsl -entry partlySetMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSET): -target wgsl -entry partlySetMain -stage compute -O0
//TEST:SIMPLE(filecheck=NOERR): -target hlsl -entry parameterPassingMain -stage compute
//TEST:SIMPLE(filecheck=NOERR): -target hlsl -entry parameterPassingMain -stage compute -O0
//TEST:SIMPLE(filecheck=NOERR): -target wgsl -entry parameterPassingMain -stage compute
//TEST:SIMPLE(filecheck=NOERR): -target wgsl -entry parameterPassingMain -stage compute -O0

// A pointer to a memory address can be declared on WGSL and HLSL without any value of it being
// read or made: a struct field that is never read, or a variable that is never assigned. Emitting
// the declaration would produce invalid code (#13399), so when no value is reported, the
// declaration is (#13632).

struct Node
{
    float4 value;
};

struct Pointers
{
    Node* nodes;
//FIELD:  ^^^^^ pointers to memory addresses are not supported on this target
//FIELD:  ^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
    uint count;
};

ConstantBuffer<Pointers> pointers;
RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void unreadFieldMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = pointers.count;
}

static Node* unassignedNode;
//GLOBAL:    ^^^^^^^^^^^^^^ pointers to memory addresses are not supported on this target
//GLOBAL:    ^^^^^^^^^^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(1, 1, 1)]
void unassignedMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = unassignedNode->value;
}

// A pointer made in a global variable's initializer is reported there, whether the initializer
// stays at module scope (HLSL) or is moved into the entry point (WGSL).
static Node* initializedNode = (Node*)uint64_t(0x1000);
//INIT:                        ^ pointers to memory addresses are not supported on this target
//INIT:                        ^ this uses a pointer to a memory address, which the current code generation target cannot represent

[shader("compute")]
[numthreads(1, 1, 1)]
void initializerMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = initializedNode->value;
}

// A struct whose pointer field is set on only some paths starts out uninitialized. Only the cast
// is reported, not the uninitialized value.
struct NodeAndCount
{
    Node* node;
    uint count;
};

RWStructuredBuffer<uint2> addresses;

[noinline]
float4 readNodeAndCount(NodeAndCount n)
{
    return n.node->value * float(n.count);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void partlySetMain(uint3 tid: SV_DispatchThreadID)
{
    NodeAndCount n;
    n.count = tid.x;
    if (tid.x > 3)
        n.node = (Node*)uint64_t(addresses[tid.x].x);
//UNSET:         ^ pointers to memory addresses are not supported on this target
//UNSET:         ^ this uses a pointer to a memory address, which the current code generation target cannot represent
    outputBuffer[tid.x] = readNodeAndCount(n);
}

// Parameter-passing types also use the `UserPointer` address space, and a local struct whose
// pointer field is never set is removed before emit. Neither is reported.
// NOERR-NOT: error[E
// NOERR: parameterPassingMain

struct PointerAndCount
{
    float* pointer;
    uint count;
};

struct Counts
{
    float scale;
    uint count;
};

groupshared float sharedValues[4];

[noinline]
void addOne(inout float x)
{
    x += 1;
}

[noinline]
void makeTwo(out float x)
{
    x = 2;
}

[noinline]
float readCount(__constref Counts c)
{
    return c.scale * float(c.count);
}

[noinline]
void addOneToFirst(inout float values[4])
{
    values[0] += 1;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void parameterPassingMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outputBuffer[tid.x].x;
    addOne(x);
    float y;
    makeTwo(y);
    PointerAndCount unset;
    unset.count = tid.x;
    Counts counts = { x, tid.x };
    sharedValues[tid.x] = x;
    GroupMemoryBarrierWithGroupSync();
    addOneToFirst(sharedValues);
    outputBuffer[tid.x] = float4(x + y + readCount(counts) + sharedValues[0] + float(unset.count));
}
