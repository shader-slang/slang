//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_WGSL): -target wgsl -entry computeMain -stage compute

// Regression test for global-varying builtins that are exposed by generic
// interface specialization after the early global-varying translation pass.

interface IWaveValue
{
    uint getValue();
}

struct WaveValue : IWaveValue
{
    uint getValue()
    {
        return WaveGetLaneIndex() + WaveGetWaveIndex() * 32;
    }
}

uint getGenericWaveValue<T: IWaveValue>(T value)
{
    return value.getValue();
}

RWStructuredBuffer<uint> outputBuffer;

// CHECK_HLSL: SV_GroupIndex
// CHECK_GLSL-DAG: GL_KHR_shader_subgroup_basic
// CHECK_GLSL-DAG: gl_SubgroupInvocationID
// CHECK_GLSL-DAG: gl_SubgroupID
// CHECK_METAL-DAG: thread_index_in_simdgroup
// CHECK_METAL-DAG: simdgroup_index_in_threadgroup
// CHECK_SPV-DAG: OpCapability GroupNonUniform
// CHECK_SPV-DAG: BuiltIn SubgroupLocalInvocationId
// CHECK_SPV-DAG: BuiltIn SubgroupId
// CHECK_WGSL-DAG: @builtin(subgroup_invocation_id)
// CHECK_WGSL-DAG: @builtin(subgroup_id)

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain()
{
    WaveValue value;
    outputBuffer[0] = getGenericWaveValue<WaveValue>(value);
}
