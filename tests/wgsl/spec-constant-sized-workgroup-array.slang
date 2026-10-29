// WGSL `var<workgroup>` arrays sized by a shift, an unsigned specialization constant, and an
// outer specialization-constant dimension over a fixed-size element (#14030).

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

// WGSL: @id(1) override kCount_0 : u32 = u32(2);
// WGSL-DAG: var<workgroup> shifted_0 : array<f32, (kTile_0 << (u32(1)))>;
// WGSL-DAG: var<workgroup> byUnsigned_0 : array<f32, i32(u32(1) + kCount_0)>;
// WGSL-DAG: var<workgroup> rows_0 : array<array<f32, i32(4)>, kTile_0>;

[SpecializationConstant]
const int kTile = 4;

[SpecializationConstant]
const uint kCount = 2;

groupshared float shifted[kTile << 1];
groupshared float byUnsigned[kCount + 1];
groupshared float rows[kTile][4];

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    shifted[gi] = gi;
    byUnsigned[gi] = gi;
    rows[gi][gi] = gi;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gi] = shifted[gi] + byUnsigned[gi] + rows[gi][gi];
}
