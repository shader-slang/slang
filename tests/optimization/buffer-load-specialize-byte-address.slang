//TEST:SIMPLE(filecheck=SPV): -target spirv -O0 -fvk-use-scalar-layout

// `byteAddressBufferLoad` has a `constexpr uint alignment` operand; specializing a callee
// through it must preserve that operand as a literal. Three calls to the same helper with
// distinct alignments (one via `Load`, two via `LoadAligned`) must yield distinct
// specializations (alignment is part of the specialization key), and each aligned load
// must carry the original alignment as a SPIR-V `Aligned` memory operand rather than a
// runtime value.

struct Frame
{
    float4x4 a;
    float4x4 b;
    float3 v;
    float pad;
};

float3 makeRay(Frame f)
{
    float4 w = mul(float4(0, 0, 0, 1), f.b);
    return w.xyz + f.v;
}

uniform ByteAddressBuffer buf;
uniform RWTexture2D<float4> outTexture;
uniform uint width;
uniform uint height;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    if (dtid.x >= width || dtid.y >= height) return;
    Frame f0 = buf.Load<Frame>(256);
    Frame f8 = buf.LoadAligned<Frame>(0, 8);
    Frame f16 = buf.LoadAligned<Frame>(128, 16);
    outTexture[dtid.xy] = float4(makeRay(f0) + makeRay(f8) + makeRay(f16), 1);
}

// One specialization of `makeRay` per alignment value (`Load` bakes in 0,
// `LoadAligned` bakes in 8 and 16). All checks are anchored to file start via
// `SPV-DAG` so the three `OpFunction` matches and the two `OpLoad` matches
// can occur in any order and in any specialization's body.
// SPV-DAG: %{{.*}} = OpFunction %v3float None
// SPV-DAG: %{{.*}} = OpFunction %v3float None
// SPV-DAG: %{{.*}} = OpFunction %v3float None
// SPV-DAG: OpLoad {{.*}} Aligned 8
// SPV-DAG: OpLoad {{.*}} Aligned 16
