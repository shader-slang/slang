// Regression test: `sample` interpolation lowers to `OpDecorate ... Sample`, which the
// SPIR-V spec gates on the SampleRateShading capability. The emitter used to write the
// decoration without declaring the capability, so the module failed spirv-val with
// "Operand 2 of Decorate requires one of these capabilities: SampleRateShading".

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry fragMain -stage fragment

struct VOut
{
    float4 pos : SV_Position;
    sample float2 uv : TEXCOORD0;
};

//SPV-DAG: OpCapability SampleRateShading
//SPV-DAG: OpDecorate %{{.*}} Sample

float4 fragMain(VOut v) : SV_Target
{
    return float4(v.uv, 0.0, 1.0);
}
