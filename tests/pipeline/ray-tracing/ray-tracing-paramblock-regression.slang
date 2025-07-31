// ray-tracing-paramblock-regression.slang

// Regression test for fix to segfault when using ParameterBlock with generics
// in ray tracing shaders

//TEST:SIMPLE(filecheck=CHECK): -target spirv

interface IMatData {}

struct PerDrawSubmesh<T> where T : IMatData
{
    T* p_mat_data;
}

struct RtUbo<T> where T : IMatData
{
    PerDrawSubmesh<T>* p_per_dsm_buff;
}

struct RtPayload
{
    float3 emission = float3(0);
}

struct RtParams<T> where T : IMatData
{
    RtUbo<T> ubo;
};

struct MatData : IMatData
{
    float4 emission;
}

// This used to cause a segfault before the fix
[shader("closesthit")]
void chit_main_with_paramblock(uniform ParameterBlock<RtParams<MatData>> params, in BuiltInTriangleIntersectionAttributes attr, out RtPayload payload)
{
    payload = {};
    payload.emission = float3(1, 0, 0);
}

// CHECK: OpEntryPoint