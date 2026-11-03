// Vulkan requires builtins whose values can change during a ray-tracing invocation to have
// volatile semantics: `RayTmaxKHR` in an intersection shader, and the subgroup builtins in
// ray generation, closest hit, miss, intersection, and callable shaders. Without the Vulkan
// memory model the variable is decorated `Volatile` (VUID-StandaloneSpirv-VulkanMemoryModel-04678);
// with it, every load uses a `Volatile` memory access instead (VUID-04679).

//TEST:SIMPLE(filecheck=GLSL450):-target spirv-asm -entry intersectionMain -stage intersection -entry rayGenMain -stage raygeneration
//TEST:SIMPLE(filecheck=VKMM):-target spirv-asm -entry intersectionMain -stage intersection -entry rayGenMain -stage raygeneration -capability vk_mem_model
//TEST:SIMPLE(filecheck=VKMM_NO_DECORATION):-target spirv-asm -entry intersectionMain -stage intersection -entry rayGenMain -stage raygeneration -capability vk_mem_model

// Stages that the VUIDs do not list get no volatile semantics: `RayTmaxKHR` in an any-hit
// shader, and the subgroup builtins in a compute shader.
//TEST:SIMPLE(filecheck=NOT_LISTED):-target spirv-asm -entry anyHitMain -stage anyhit
//TEST:SIMPLE(filecheck=NOT_LISTED):-target spirv-asm -entry computeMain -stage compute

// A compute and a ray generation entry point that both read `WaveGetLaneIndex()` share one
// `SubgroupLocalInvocationId` variable, which must be volatile because of the ray generation use.
//TEST:SIMPLE(filecheck=SHARED):-target spirv-asm -entry computeMain -stage compute -entry rayGenMain -stage raygeneration

// GLSL450-DAG: OpDecorate %[[TMAX:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// GLSL450-DAG: OpDecorate %[[TMAX]] Volatile
// GLSL450-DAG: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// GLSL450-DAG: OpDecorate %[[LANE]] Volatile
// GLSL450-DAG: OpDecorate %[[SMID:[A-Za-z0-9_]+]] BuiltIn SMIDNV
// GLSL450-DAG: OpDecorate %[[SMID]] Volatile

// VKMM: OpMemoryModel Logical Vulkan
// VKMM-DAG: OpDecorate %[[TMAX:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// VKMM-DAG: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// VKMM-DAG: OpDecorate %[[SMID:[A-Za-z0-9_]+]] BuiltIn SMIDNV
// VKMM-DAG: OpLoad %float %[[TMAX]] Volatile
// VKMM-DAG: OpLoad %uint %[[LANE]] Volatile
// VKMM-DAG: OpLoad %uint %[[SMID]] Volatile

// VKMM_NO_DECORATION-NOT: OpDecorate {{.*}} Volatile

// NOT_LISTED: OpEntryPoint
// NOT_LISTED-NOT: Volatile
// NOT_LISTED: OpFunctionEnd

// SHARED: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// SHARED-NOT: OpDecorate %[[LANE]] BuiltIn
// SHARED: OpDecorate %[[LANE]] Volatile

struct Attributes
{
    float t;
};

RWStructuredBuffer<uint> output;

[shader("intersection")]
void intersectionMain()
{
    Attributes attr;
    attr.t = RayTCurrent();
    ReportHit(RayTCurrent() * 0.5, 0, attr);
}

[shader("anyhit")]
void anyHitMain(inout float payload, Attributes attr)
{
    payload = RayTCurrent();
}

[shader("raygeneration")]
void rayGenMain()
{
    // Slang has no intrinsic for `SMIDNV`, so we read it through inline SPIR-V.
    uint smid = spirv_asm {
        OpExtension "SPV_NV_shader_sm_builtins";
        OpCapability ShaderSMBuiltinsNV;
        result:$$uint = OpLoad builtin(SMIDNV:uint);
    };
    output[DispatchRaysIndex().x] = WaveGetLaneIndex() + smid;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[tid.x] = WaveGetLaneIndex();
}
