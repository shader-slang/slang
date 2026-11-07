//TEST:SIMPLE(filecheck=DISTINCT): -target wgsl -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DEFS): -target wgsl -obfuscate -entry computeMain -stage compute

// An original that survives address-space specialization next to its only specialization
// (#14004). No entry point calls the exported `notAnEntry`, so the pass does not visit it, and its
// call keeps the original `bump` alive. `computeMain` gives `bump` one `function` specialization.
// The original keeps its linkage name, so the specialization must lose it: WGSL has no
// overloading and rejects two functions with one name.

// DISTINCT: fn [[BUMP:[_A-Za-z0-9]+]]({{.*}}ptr<function, u32>
// DISTINCT-NOT: fn [[BUMP]](

// DEFS-COUNT-2: fn {{[_A-Za-z0-9]+}}({{.*}}ptr<function, u32>
// DEFS-NOT: fn {{[_A-Za-z0-9]+}}({{.*}}ptr<function, u32>

RWStructuredBuffer<uint> result;

void bump(inout uint dst, uint v)
{
    dst += v;
}

export void notAnEntry()
{
    uint x = 0;
    bump(x, 1);
    result[1] = x;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    uint local = id.x;
    bump(local, 1);
    result[id.x] = local;
}
