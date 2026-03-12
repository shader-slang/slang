// Test that obfuscation does not incorrectly coalesce two phi-node variables
// that carry distinct values (rotationQuatT0 and rotationQuatT1) into a single
// OpVariable, which would cause FMix to receive the same value for both inputs.
//
// Regression test for a bug where stripping IRNameHintDecoration (obfuscation)
// caused the SSA register allocator to merge two phi params in different blocks
// into one variable, because the name-based interference guard was gone and
// isInstReachable failed to detect the conflict through nested SPIRVAsmOperandInst.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -O0 -obfuscate

// Two distinct OpVariables must be allocated for the two phi nodes (t0 and t1).
// If the bug is present, only one variable is allocated and the second CHECK fails.
// CHECK: %[[V0:[0-9]+]] = OpVariable %_ptr_Function_v4float Function
// CHECK: %[[V1:[0-9]+]] = OpVariable %_ptr_Function_v4float Function
// Loads from the two distinct variables must feed FMix with different values.
// CHECK: %[[L0:[0-9]+]] = OpLoad %v4float %[[V0]]
// CHECK: %[[L1:[0-9]+]] = OpLoad %v4float %[[V1]]
// CHECK-NOT: FMix %[[L0]] %[[L0]]
// CHECK: FMix %[[L0]] %[[L1]]

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
    // Two separate ternary results: each becomes a distinct phi node.
    // After obfuscation (name hints stripped), the register allocator
    // must not coalesce these two into the same variable.
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
