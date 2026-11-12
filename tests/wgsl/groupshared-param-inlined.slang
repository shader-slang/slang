// WGSL cannot take a `ptr<workgroup, ...>` as a function parameter, so a function with a
// `groupshared` parameter is inlined into its callers, and the accesses land on the `workgroup`
// global directly, never on a `function`-space copy of it (shader-slang/slang#10641). The
// execution run is in tests/language-feature/groupshared-param/by-reference.slang.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=FIELD): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

struct Slots
{
    uint v[N];
}
groupshared Slots groupSharedSlots;

void writeSlot(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
}

// WGSL cannot form a pointer to part of a composite, so if this call survived to
// `legalizeIRForWGSL`, its `legalizeCall` would bridge the field argument through a
// `function`-space temporary.
void writeField(uint tid, groupshared uint slot)
{
    slot = tid + 1;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    writeSlot(tid.x, groupSharedData);
    writeField(tid.x, groupSharedSlots.v[tid.x]);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}

// WGSL-NOT: ptr<workgroup
// WGSL-NOT: var {{[A-Za-z0-9_]+}} : array<u32,{{.*}}= groupSharedData
// WGSL: groupSharedData{{.*}}[{{.*}}] =

// FIELD-NOT: ptr<workgroup
// FIELD-NOT: var {{[A-Za-z0-9_]+}} : u32 = groupSharedSlots
// FIELD: groupSharedSlots{{[_0-9]*}}.v{{.*}}] =
