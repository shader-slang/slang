// Vulkan requires builtins whose values can change during a ray-tracing invocation to have
// volatile semantics: `RayTmaxKHR` in an intersection shader, and the subgroup builtins in
// ray generation, closest hit, miss, intersection, and callable shaders. Without the Vulkan
// memory model the variable is decorated `Volatile` (VUID-StandaloneSpirv-VulkanMemoryModel-04678);
// with it, every load uses a `Volatile` memory access instead (VUID-04679).

//TEST:SIMPLE(filecheck=GLSL450):-target spirv-asm -entry intersectionMain -stage intersection -entry rayGenMain -stage raygeneration
//TEST:SIMPLE(filecheck=VKMM):-target spirv-asm -entry intersectionMain -stage intersection -entry rayGenMain -stage raygeneration -capability vk_mem_model

// GLSL450-DAG: OpDecorate %[[TMAX:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// GLSL450-DAG: OpDecorate %[[TMAX]] Volatile
// GLSL450-DAG: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// GLSL450-DAG: OpDecorate %[[LANE]] Volatile

// VKMM: OpMemoryModel Logical Vulkan
// VKMM-NOT: Volatile{{$}}
// VKMM-DAG: OpDecorate %[[TMAX:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// VKMM-DAG: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// VKMM-DAG: OpLoad %float %[[TMAX]] Volatile
// VKMM-DAG: OpLoad %uint %[[LANE]] Volatile

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

[shader("raygeneration")]
void rayGenMain()
{
    output[DispatchRaysIndex().x] = WaveGetLaneIndex();
}
