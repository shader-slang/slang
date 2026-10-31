//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6 -O2
//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6 -O0

// Regression test for #13948, using the reporter's amplification shader. `inside`
// receives `r.view`, a pointer field of the local copy `r`, and `View` needs a
// storage layout type because of its array field. Compared with
// ptr-field-from-local-copy.slang, this file adds the task stage, a copy through
// the push-constant dereference `*push.root`, and both -O0 and -O2. The check
// passes once SPIR-V is produced, so it guards the `emitFieldAddress` assert;
// SLANG_RUN_SPIRV_VALIDATION=1 also validates the output.

// CHECK: OpEntryPoint TaskEXT

struct RootPtr<T>
{
    T* root;
};
struct View
{
    float4 planes[6];
    float4 eye;
};
struct Mesh
{
    float4* bounds;
    uint count;
};
struct Root
{
    View* view;
    Mesh* meshes;
    uint mesh;
};
[[vk::push_constant]] ConstantBuffer<RootPtr<Root>> push;
struct Payload
{
    uint ids[32];
};
groupshared Payload payload;

bool inside(View* v, float4 b)
{
    bool ok = true;
    for (uint i = 0; i < 6; i++)
    {
        float4 p = v.planes[i];
        ok = ok && dot(p.xyz, b.xyz) + p.w >= -b.w;
    }
    return ok;
}

[shader("amplification")]
[numthreads(32, 1, 1)]
void ts(uint lane: SV_GroupThreadID, uint group: SV_GroupID)
{
    Root r = *push.root;
    Mesh m = r.meshes[r.mesh];
    uint k = group * 32 + lane;
    bool seen = false;
    if (k < m.count)
    {
        seen = inside(r.view, m.bounds[k]);
    }
    uint at = WavePrefixCountBits(seen);
    if (seen)
    {
        payload.ids[at] = k;
    }
    DispatchMesh(WaveActiveCountBits(seen), 1, 1, payload);
}
