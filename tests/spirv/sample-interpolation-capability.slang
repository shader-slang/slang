// Test that `sample` interpolation declares OpCapability SampleRateShading.
//
// The shader deliberately uses no sample-rate system value: SV_SampleIndex and
// SV_VulkanSamplePosition declare that capability themselves, which would let this
// test pass even while the interpolation path emits an undeclared decoration.

//TEST:SIMPLE(filecheck=CHECK-SPIRV):-target spirv -entry main -stage fragment

struct VsOut
{
    float4 pos : SV_Position;
    sample float4 perSample : COLOR0;
};

float4 main(VsOut v) : SV_Target
{
    return v.perSample;
}

// CHECK-SPIRV-DAG: OpCapability SampleRateShading
// CHECK-SPIRV-DAG: OpDecorate %{{.*}} Sample
