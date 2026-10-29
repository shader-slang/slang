//TEST_IGNORE_FILE:

// module-library-address-space-specialization.slang

module "module-library-address-space-specialization";

public RWStructuredBuffer<float> result;

public void bump(inout float dst, float v)
{
    dst += v;
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

// The entry point makes address-space specialization clone the exported `bump` and `Acc.add`
// while the module is precompiled.
[shader("compute")]
[numthreads(64, 1, 1)]
void libMain(uint3 id: SV_DispatchThreadID)
{
    float local = id.x;
    bump(local, 1);
    bump(gSharedValues[id.x], 2);

    Acc acc = {};
    acc.add(1);
    gPrivate.add(2);
    gShared.add(3);

    GroupMemoryBarrierWithGroupSync();
    result[id.x] = local + gSharedValues[id.x] + acc.v + gPrivate.v + gShared.v;
}
