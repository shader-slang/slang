//TEST:SIMPLE(filecheck=DISTINCT): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLONES): -target metal -obfuscate -entry computeMain -stage compute

// An exported function called with a `thread` and a `threadgroup` pointer (#14004). `export`
// adds `HLSLExport` and `[KeepAlive]` to `bump`, and its specializations copy them. Whether the
// exported original survives the pass or not, its emitted definitions must have different names
// under `-obfuscate`, where a definition with a linkage name is named by it.

// DISTINCT: {{^}}void [[BUMP:[_A-Za-z0-9]+]](uint {{thread|threadgroup}}*
// DISTINCT-NOT: {{^}}void [[BUMP]](

// CLONES-COUNT-2: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*
// CLONES-NOT: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*

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
