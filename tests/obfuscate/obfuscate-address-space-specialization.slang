//TEST:SIMPLE(filecheck=DISTINCT): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLONES): -target metal -obfuscate -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SINGLE): -target metal -obfuscate -entry computeMainSingle -stage compute
//TEST:SIMPLE(filecheck=LOCATE-WGSL): -target wgsl -obfuscate -entry computeMainLocate -stage compute
//TEST:SIMPLE(filecheck=LOCATE-METAL): -target metal -obfuscate -entry computeMainLocate -stage compute

// Address-space specialization clones a function once per set of pointer-argument address
// spaces (#14004). Under `-obfuscate` there are no name hints, so an emitter names a function by
// its linkage name, and two surviving clones of `bump` must not share the original's linkage
// name. Metal emits one `thread*` and one `threadgroup*` clone, which must get different names.

// DISTINCT: {{^}}void [[BUMP:[_A-Za-z0-9]+]](uint {{thread|threadgroup}}*
// DISTINCT-NOT: {{^}}void [[BUMP]](

// CLONES-COUNT-2: {{^}}void {{[_A-Za-z0-9]+}}(uint {{thread|threadgroup}}*

// Without `-obfuscate` the clones are named from their name hints, as before.

// PLAIN-DAG: {{^}}void bump_{{[0-9]+}}(uint thread*
// PLAIN-DAG: {{^}}void bump_{{[0-9]+}}(uint threadgroup*

// A single clone that replaces its removed original keeps the original's linkage name.

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

// The reporter's shader: the five-argument `locate` is reached from two call paths, which
// share one specialization. Each emitted function has its own name, and the specializations of
// `locate` and `classify` keep the linkage names of the originals they replace, including
// `classify`, whose original is only called from another removed original.

// LOCATE-WGSL: fn [[CLASSIFY:_Sh[0-9A-F]+]]({{.*}}: vec3<i32>, {{.*}}) -> u32
// LOCATE-WGSL: fn [[LOCATE:_Sh[0-9A-F]+]]({{.*}}: f32, {{.*}}: i32) -> vec2<u32>
// LOCATE-WGSL-NOT: fn [[CLASSIFY]](
// LOCATE-WGSL-NOT: fn [[LOCATE]](

// LOCATE-METAL: {{^}}uint [[CLASSIFY:_Sh[0-9A-F]+]](float3 {{.*}}, int3
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
