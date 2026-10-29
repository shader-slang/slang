//TEST:SIMPLE(filecheck=DISTINCT): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLONES): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DISTINCT-WGSL): -target wgsl -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SINGLE): -target metal -obfuscate -entry computeMainSingle -stage compute
//TEST:SIMPLE(filecheck=LOCATE-WGSL): -target wgsl -obfuscate -entry computeMainLocate -stage compute
//TEST:SIMPLE(filecheck=LOCATE-METAL): -target metal -obfuscate -entry computeMainLocate -stage compute

// Address-space specialization clones a function once per set of pointer-argument address
// spaces (#14004). Under `-obfuscate` there are no name hints, so an emitter names a function
// with a linkage name by that name, which is a hash such as `_Sh2F0590F5ACC4C8A8`, and names
// a function without one `_S<n>`. The two surviving clones of `bump`, one `thread*` and one
// `threadgroup*` on Metal, must not share the original's linkage name. The DISTINCT checks
// capture the first definition's name and reject any later definition with that name.

// DISTINCT: {{^}}void [[BUMP:[_A-Za-z0-9]+]](uint {{thread|threadgroup}}*
// DISTINCT-NOT: {{^}}void [[BUMP]](

// CLONES-COUNT-2: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*
// CLONES-NOT: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*

// WGSL copies the `groupshared` element through a `function` temporary before the call, so `bump`
// has a single clone there; the WGSL run checks that its definitions stay uniquely named.

// DISTINCT-WGSL: fn [[BUMP:[_A-Za-z0-9]+]]({{.*}}ptr<{{function|workgroup}}
// DISTINCT-WGSL-NOT: fn [[BUMP]](

// Without `-obfuscate`, the clones are named from their name hints.

// PLAIN-DAG: {{^}}void bump_{{[0-9]+}}(uint thread*
// PLAIN-DAG: {{^}}void bump_{{[0-9]+}}(uint threadgroup*

// A single clone that replaces its removed original keeps the original's linkage name, so its
// name is a hash (`_Sh`) rather than `_S<n>`.

// SINGLE: {{^}}void _Sh{{[0-9A-F]+}}(uint thread*

RWStructuredBuffer<uint> result;
groupshared uint sharedValues[64];

void bump(inout uint dst, uint v)
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

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMainSingle(uint3 id: SV_DispatchThreadID)
{
    uint local = id.x;
    bump(local, 1);
    result[id.x] = local;
}

// An overload set whose widest overload takes an `out` parameter, reached from two call paths:
// `locate(p)` calls the four-argument `locate`, which calls the five-argument `locate`, and
// `locateSample` calls the five-argument `locate` directly. Both paths share one specialization of
// the five-argument `locate`. Each emitted function has its own name, and the specializations of
// `locate` and `classify` keep the hashed linkage names of the originals they replace. The
// original `classify` is only called from the original five-argument `locate`, so it is removed
// only once that original is removed, which the repeated removal in
// `removeDeadRedirectedCallees` handles.

// LOCATE-WGSL: fn [[CLASSIFY:_Sh[0-9A-F]+]]({{.*}}: vec3<i32>, {{.*}}) -> u32
// LOCATE-WGSL-NOT: fn [[CLASSIFY]](
// LOCATE-WGSL: fn [[LOCATE:_Sh[0-9A-F]+]]({{.*}}: f32, {{.*}}: i32) -> vec2<u32>
// LOCATE-WGSL-NOT: fn [[CLASSIFY]](
// LOCATE-WGSL-NOT: fn [[LOCATE]](

// LOCATE-METAL: {{^}}uint [[CLASSIFY:_Sh[0-9A-F]+]](float3 {{.*}}, int3
// LOCATE-METAL-NOT: {{^}}uint [[CLASSIFY]](
// LOCATE-METAL: {{^}}uint2 [[LOCATE:_Sh[0-9A-F]+]](float3 {{.*}}, float {{.*}}, int {{.*}}, KernelContext
// LOCATE-METAL-NOT: {{^}}uint [[CLASSIFY]](
// LOCATE-METAL-NOT: {{^}}uint2 [[LOCATE]](

StructuredBuffer<float4> positions;
StructuredBuffer<uint> mode;
uniform float3 origin, spacing;

uint classify(float3 p, int3 cell, out uint2 extra)
{
    extra = uint2(cell.xy);
    return uint(p.x);
}

uint2 locate(float3 p, int radius, out uint2 extra, float tolerance, int widen)
{
    int3 cell = int3((p - origin) / spacing);
    uint c = classify(p, cell, extra);
    return uint2(c, uint(widen));
}

uint2 locate(float3 p, int radius, out uint2 extra, float tolerance)
{
    return locate(p, radius, extra, tolerance, radius);
}

uint2 locate(float3 p)
{
    uint2 extra;
    return locate(p, 1, extra, 8);
}

uint2 locateSample(float3 p)
{
    uint2 extra;
    return locate(p, 1, extra, 0, 3);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMainLocate(uint3 id: SV_DispatchThreadID)
{
    uint2 r = mode[id.x] != 0 ? locateSample(positions[id.x].xyz) : locate(positions[id.x].xyz);
    result[id.x] = r.x;
}
