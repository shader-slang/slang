// Test that obfuscation preserves the two distinct phi values consumed by
// FMix. Their uses are nested inside SPIRVAsmOperandInst instructions, so
// optimizations must account for those uses after stripping name hints.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -obfuscate

// The direct SPIR-V backend preserves the two vector phis instead of
// coalescing them into mutable temporaries.
// CHECK: %[[T0:[0-9]+]] = OpPhi %v4float
// CHECK: %[[T1:[0-9]+]] = OpPhi %v4float
// CHECK: FMix %[[T0]] %[[T1]]

struct Params
{
    float4 valT0;
    float4 valT1;
    float4 prevValT0;
    float4 prevValT1;
};

ParameterBlock<Params> gParams;
RWStructuredBuffer<float4> gOutput;

float4 getT0(Params p) { return p.valT0; }
float4 getT1(Params p) { return p.valT1; }
float4 getPrevT0(Params p) { return p.prevValT0; }
float4 getPrevT1(Params p) { return p.prevValT1; }

float4 interpolate(Params p, float time, bool current)
{
    // The ternary results must remain distinct SSA values after obfuscation
    // strips their name hints, so FMix receives the two different inputs.
    float4 t0 = current ? getT0(p) : getPrevT0(p);
    float4 t1 = current ? getT1(p) : getPrevT1(p);
    return normalize(lerp(t0, t1, time));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Params p = gParams;
    gOutput[tid.x] = interpolate(p, 0.5f, true);
}
