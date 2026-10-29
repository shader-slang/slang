// module-library-address-space-specialization.slang

// A library for precompiled-spirv-address-space-specialization.slang (#14004). Its entry point
// specializes the exported `bump`, `inc` and `Acc.add` while the module is precompiled. The
// exported originals stay, and their specializations lose their linkage, so the precompiled
// SPIR-V exports each name exactly once. The checks capture each export name and reject a second
// `LinkageAttributes` decoration with it before the first function definition.

//TEST:SIMPLE(filecheck=LIB): -o tests/library/module-library-address-space-specialization-dump.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name -dump-ir

// LIB: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[BUMP:[^"]*4bumpp[^"]*]]" Export
// LIB-NOT: LinkageAttributes "[[BUMP]]" Export
// LIB: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[INC:[^"]*3incp[^"]*]]" Export
// LIB-NOT: LinkageAttributes "{{[^"]*}}{{4bumpp|3incp}}{{[^"]*}}" Export
// LIB: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[ADD:[^"]*3Acc3addp[^"]*]]" Export
// LIB-NOT: LinkageAttributes "{{[^"]*}}{{4bumpp|3incp|3Acc3addp}}{{[^"]*}}" Export
// LIB: = OpFunction

module "module-library-address-space-specialization";

public RWStructuredBuffer<float> result;

public void bump(inout float dst, float v)
{
    dst += v;
}

// Called only with a local, so it keeps its original (exported) and gets one specialization.
public void inc(inout float dst)
{
    dst += 1;
}

public struct Acc
{
    public float v;

    [mutating]
    public void add(float x)
    {
        v += x;
    }
};

static Acc gPrivate;
groupshared Acc gShared;
groupshared float gSharedValues[64];

// `bump` is specialized for a local and a `groupshared` element, `Acc.add` for a local, a
// `static` and a `groupshared` receiver, and `inc` for a local only.
[shader("compute")]
[numthreads(64, 1, 1)]
void libMain(uint3 id: SV_DispatchThreadID)
{
    float local = id.x;
    bump(local, 1);
    bump(gSharedValues[id.x], 2);
    inc(local);

    Acc acc = {};
    acc.add(1);
    gPrivate.add(2);
    gShared.add(3);

    GroupMemoryBarrierWithGroupSync();
    result[id.x] = local + gSharedValues[id.x] + acc.v + gPrivate.v + gShared.v;
}
