//TEST:SIMPLE(filecheck=CHECK_SPIRV): -entry main -stage compute -target spirv -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_GLSL): -entry main -stage compute -target glsl -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_HLSL): -entry main -stage compute -target hlsl -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_METAL): -entry main -stage compute -target metal -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_WGSL): -entry main -stage compute -target wgsl -allow-glsl

RWStructuredBuffer<uint> outputBuffer;

T getGlobalInvocationID<T: IInteger>(T value)
{
    return T(gl_GlobalInvocationID.x) + value;
}

T getWaveLaneIndex<T: IInteger>(T value)
{
    return T(WaveGetLaneIndex()) + value;
}

// CHECK_SPIRV-DAG: GlobalInvocationId
// CHECK_SPIRV-DAG: SubgroupLocalInvocationId

// CHECK_GLSL-DAG: gl_GlobalInvocationID
// CHECK_GLSL-DAG: gl_SubgroupInvocationID

// CHECK_HLSL-DAG: WaveGetLaneIndex()
// CHECK_HLSL-DAG: SV_DispatchThreadID

// CHECK_METAL-DAG: thread_position_in_grid
// CHECK_METAL-DAG: thread_index_in_simdgroup

// CHECK_WGSL-DAG: global_invocation_id
// CHECK_WGSL-DAG: subgroup_invocation_id

[shader("compute")]
void main()
{
    outputBuffer[0U] = getGlobalInvocationID(0U);
    outputBuffer[1U] = getWaveLaneIndex(0U);
}


