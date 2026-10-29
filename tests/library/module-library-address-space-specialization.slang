// module-library-address-space-specialization.slang

// A library for precompiled-spirv-address-space-specialization.slang (#14004). Its entry point
// specializes the exported `bump`, `inc` and `Acc.add` while the module is precompiled. The
// exported originals stay, and their specializations lose their linkage, so the precompiled
// SPIR-V exports each name exactly once.
//
// `-dump-ir` prints the embedded SPIR-V as disassembly; the `.slang-module` these runs write is
// not used. Each run captures the first `LinkageAttributes` export of one function and rejects a
// second one for the same name before the first function definition, in any order.

//TEST:SIMPLE(filecheck=BUMP): -o tests/library/module-library-address-space-specialization-bump.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name -dump-ir
//TEST:SIMPLE(filecheck=INC): -o tests/library/module-library-address-space-specialization-inc.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name -dump-ir
//TEST:SIMPLE(filecheck=ADD): -o tests/library/module-library-address-space-specialization-add.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name -dump-ir

// BUMP: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[NAME:[^"]*4bumpp[^"]*]]" Export
// BUMP-NOT: LinkageAttributes "[[NAME]]" Export
// BUMP: = OpFunction

// INC: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[NAME:[^"]*3incp[^"]*]]" Export
// INC-NOT: LinkageAttributes "[[NAME]]" Export
// INC: = OpFunction

// ADD: OpDecorate %{{[A-Za-z_0-9]+}} LinkageAttributes "[[NAME:[^"]*3Acc3addp[^"]*]]" Export
// ADD-NOT: LinkageAttributes "[[NAME]]" Export
// ADD: = OpFunction

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
