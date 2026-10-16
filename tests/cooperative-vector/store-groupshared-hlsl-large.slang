// Verify that the `N > __kMaxUnrolledElements` test in the CoopVec groupshared store/load is
// constant folded: for a vector longer than the unroll limit only the plain loop survives. The
// length is above the 4096-iteration cap of [ForceUnroll], so if the dead [ForceUnroll] copy were
// still present when the loop unroller ran, compilation would fail with an un-unrollable loop.
// The test needs a length above both that cap and `__kMaxUnrolledElements`; it does not depend on
// the exact value of either.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile cs_6_10 -entry main_cs -stage compute

groupshared half s_Stage[8192];

RWByteAddressBuffer u_Out;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_cs(uint gi : SV_GroupIndex)
{
    CoopVec<half, 8192> v = CoopVec<half, 8192>(half(gi));
    v.store(s_Stage, 0);
    GroupMemoryBarrierWithGroupSync();
    CoopVec<half, 8192> w = CoopVec<half, 8192>.load(s_Stage, 0);
    u_Out.Store(0, asuint(float(w[5])));
}

// HLSL: for(;;)
// HLSL: if(i_0 < int(8192))
// HLSL: s_Stage_0[i_0] = {{.*}}[i_0];
