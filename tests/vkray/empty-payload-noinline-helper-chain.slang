// The decorated payload globals reach the actual SPIR-V instruction through three helpers.
// None of the helpers is a public TraceRay or CallShader wrapper. Their branches and atomic
// writes must survive, while ordinary empty parameters disappear from every helper signature.
// Only the innermost dispatch needs a synthetic interface object. Finding that boundary must
// not require inlining the helpers or propagating a physical carrier through their signatures.
// Use __ref plus noRefInline to opt out of the pre-existing reference-parameter inliner as well.

//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O0 -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O3 -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O0 -DUSE_CALLABLE=1 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -O3 -DUSE_CALLABLE=1 -validate-ir

#ifndef USE_CALLABLE
#define USE_CALLABLE 0
#endif

struct Empty
{
}

struct Payload
{
    Empty values[2];
}

[__vulkanRayPayload]
static Payload rayPayload;

[__vulkanCallablePayload]
static Empty callablePayload[2];

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> events;

[noinline]
[noRefInline]
void dispatchLeaf(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[0], tag);
#if USE_CALLABLE
    uint shaderIndex = 0;
    spirv_asm
    {
        OpCapability RayTracingKHR;
        OpExtension "SPV_KHR_ray_tracing";
        OpExecuteCallableKHR $shaderIndex &callableData;
    };
#else
    uint flags = 0;
    uint mask = 0xff;
    uint offset = 0;
    float3 origin = float3(0.0);
    float3 direction = float3(0.0, 0.0, 1.0);
    float tMin = 0.0;
    float tMax = 1.0;
    spirv_asm
    {
        OpCapability RayTracingKHR;
        OpExtension "SPV_KHR_ray_tracing";
        OpTraceRayKHR $scene $flags $mask $offset $offset $offset
            $origin $tMin $direction $tMax &rayData;
    };
#endif
    InterlockedAdd(events[1], tag);
}

[noinline]
[noRefInline]
void forwardMiddle(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[2], tag);
    if ((tag & 1) != 0)
    {
        InterlockedAdd(events[6], tag);
        dispatchLeaf(rayData, callableData, tag + 1);
    }
    else
        dispatchLeaf(rayData, callableData, tag + 2);
    InterlockedAdd(events[3], tag);
}

[noinline]
[noRefInline]
void forwardOuter(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[4], tag);
    forwardMiddle(rayData, callableData, tag);
    InterlockedAdd(events[5], tag);
}

[shader("raygeneration")]
void main()
{
    forwardOuter(rayPayload, callablePayload, DispatchRaysIndex().x);
}

// Every helper has the same void(uint) signature after both empty payload arguments disappear.
// Capture the real interface variable, rather than accepting any pointer passed to the dispatch.
// SPIRV-DAG: OpName %[[outer:[A-Za-z0-9_]+]] "forwardOuter"
// SPIRV-DAG: OpName %[[middle:[A-Za-z0-9_]+]] "forwardMiddle"
// SPIRV-DAG: OpName %[[leaf:[A-Za-z0-9_]+]] "dispatchLeaf"
// SPIRV-DAG: %[[void:[A-Za-z0-9_]+]] = OpTypeVoid
// SPIRV-DAG: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV-DAG: %[[signature:[A-Za-z0-9_]+]] = OpTypeFunction %[[void]] %[[uint]]{{$}}
// SPIRV-DAG: %[[payloadPointer:[A-Za-z0-9_]+]] = OpTypePointer {{RayPayloadKHR|CallableDataKHR}} %{{[A-Za-z0-9_]+}}
// SPIRV-DAG: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[payloadPointer]] {{RayPayloadKHR|CallableDataKHR}}

// SPIRV: OpFunctionCall %[[void]] %[[outer]] %{{[A-Za-z0-9_]+}}{{$}}
// SPIRV: %[[outer]] = OpFunction %[[void]] DontInline %[[signature]]
// SPIRV: OpAtomicIAdd
// SPIRV: OpFunctionCall %[[void]] %[[middle]] %{{[A-Za-z0-9_]+}}{{$}}
// SPIRV: OpAtomicIAdd
// SPIRV: OpFunctionEnd

// SPIRV: %[[middle]] = OpFunction %[[void]] DontInline %[[signature]]
// SPIRV: OpAtomicIAdd
// SPIRV: OpBranchConditional
// SPIRV: OpFunctionCall %[[void]] %[[leaf]] %{{[A-Za-z0-9_]+}}{{$}}
// SPIRV: OpFunctionCall %[[void]] %[[leaf]] %{{[A-Za-z0-9_]+}}{{$}}
// SPIRV: OpAtomicIAdd
// SPIRV: OpFunctionEnd

// SPIRV: %[[leaf]] = OpFunction %[[void]] DontInline %[[signature]]
// SPIRV: OpAtomicIAdd
// SPIRV: Op{{TraceRayKHR|ExecuteCallableKHR}} {{.*}} %[[payload]]{{$}}
// SPIRV: OpAtomicIAdd
// SPIRV: OpFunctionEnd
