// Verify that storing a CoopVec to groupshared memory on HLSL unrolls into constant-index
// element stores rather than a loop that dynamically indexes the vector, and that the
// `N > __kMaxUnrolledElements` test is constant folded so no trace of it remains.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile cs_6_10 -entry main_cs -stage compute

groupshared half s_Stage[64 * 48];

RWByteAddressBuffer u_Out;

[shader("compute")]
[numthreads(64, 1, 1)]
void main_cs(uint gi : SV_GroupIndex)
{
    CoopVec<half, 4> v = CoopVec<half, 4>(half(gi));
    v.store(s_Stage, int(gi * 96));
    GroupMemoryBarrierWithGroupSync();
    u_Out.Store(gi * 4, asuint(float(s_Stage[(gi * 48 + 5) % (64 * 48)])));
}

// HLSL-NOT: for(
// HLSL-NOT: if (
// HLSL: s_Stage_0[{{.*}}] = {{.*}}[int(0)];
// HLSL: s_Stage_0[{{.*}}] = {{.*}}[int(1)];
// HLSL: s_Stage_0[{{.*}}] = {{.*}}[int(2)];
// HLSL: s_Stage_0[{{.*}}] = {{.*}}[int(3)];
