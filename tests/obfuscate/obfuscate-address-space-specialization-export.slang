//TEST:SIMPLE(filecheck=DISTINCT): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLONES): -target metal -obfuscate -entry computeMain -stage compute

// An exported function called with a `thread` and a `threadgroup` pointer (#14004). Its emitted
// definitions must have different names under `-obfuscate`, where every definition is otherwise
// named by the function's one linkage name.

// DISTINCT: {{^}}void [[BUMP:[_A-Za-z0-9]+]](uint {{thread|threadgroup}}*
// DISTINCT-NOT: {{^}}void [[BUMP]](

// CLONES-COUNT-2: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*

RWStructuredBuffer<uint> result;
groupshared uint sharedValues[64];

export void bump(inout uint dst, uint v)
{
    dst += v;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    uint local = id.x;
    bump(local, 1);
    bump(sharedValues[id.x], 3);
    GroupMemoryBarrierWithGroupSync();
    result[id.x] = local + sharedValues[id.x];
}
