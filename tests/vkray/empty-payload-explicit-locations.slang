// Ray payloads and callable data have separate location namespaces. Both empty objects below use
// location 7 and are referenced only by late-resolved assembly operands, not ordinary pointers.
// Type legalization must preserve their binding identity when replacing erased logical storage.

//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O3 -validate-ir

struct Empty
{
}

[__vulkanRayPayload(7)]
static Empty rayPayload;

[__vulkanCallablePayload(7)]
static Empty callablePayload;

RaytracingAccelerationStructure scene;

[shader("raygeneration")]
void main()
{
    uint zero = 0;
    uint mask = 0xff;
    float3 origin = float3(0.0);
    float3 direction = float3(0.0, 0.0, 1.0);
    float tMin = 0.0;
    float tMax = 1.0;
    spirv_asm
    {
        OpCapability RayTracingKHR;
        OpExtension "SPV_KHR_ray_tracing";
        OpName __rayPayloadFromLocation(7) "empty_ray_at_seven";
        OpName __rayCallableFromLocation(7) "empty_callable_at_seven";
        OpTraceRayKHR $scene $zero $mask $zero $zero $zero
            $origin $tMin $direction $tMax __rayPayloadFromLocation(7);
        OpExecuteCallableKHR $zero __rayCallableFromLocation(7);
    };
}

// The names and locations identify different globals even though their numeric locations match.
// SPIRV-DAG: OpName %[[ray:[A-Za-z0-9_]+]] "empty_ray_at_seven"
// SPIRV-DAG: OpName %[[callable:[A-Za-z0-9_]+]] "empty_callable_at_seven"
// SPIRV-DAG: OpDecorate %[[ray]] Location 7
// SPIRV-DAG: OpDecorate %[[callable]] Location 7
// SPIRV-DAG: %[[ray]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// SPIRV-DAG: %[[callable]] = OpVariable %{{[A-Za-z0-9_]+}} CallableDataKHR
// SPIRV: OpTraceRayKHR {{.*}} %[[ray]]
// SPIRV: OpExecuteCallableKHR %{{[A-Za-z0-9_]+}} %[[callable]]
