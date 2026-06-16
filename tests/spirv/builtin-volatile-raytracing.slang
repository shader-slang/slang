// builtin-volatile-raytracing.slang
//
// Regression test for shader-slang/slang#10528. Subgroup builtins used
// in any of the five raytracing-volatile stages (raygen, closesthit,
// miss, intersection, callable) must be decorated `Volatile` under the
// GLSL450 memory model — invocation repacking can change the underlying
// value between accesses. Per-stage directives pin
// `isRaytracingStageRequiringVolatileBuiltins` so a regression that
// drops one switch arm fails. The raygen entry point also exercises
// the via-glsl path (glslang owns the decoration in `Initialize.cpp`'s
// `rtSubgroupDecls` block) and the SPIR-V emission of several wave
// intrinsics — enough to catch a regression in either the direct emit
// path or the glslang baseline.

//TEST:SIMPLE(filecheck=CHECK_RAYGEN): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_CLOSESTHIT): -target spirv-asm -entry closestHitMain -stage closesthit -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MISS): -target spirv-asm -entry missMain -stage miss -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_INTERSECTION): -target spirv-asm -entry intersectionMain -stage intersection -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_CALLABLE): -target spirv-asm -entry callableMain -stage callable -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_VIA_GLSL): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-via-glsl

RWStructuredBuffer<uint> output;

struct Payload { uint v; };
struct Attribs { float2 b; };
struct CallableData { uint v; };

// Direct emit, raygen — covers wave intrinsic emission alongside the Volatile decoration.
// CHECK_RAYGEN-DAG: OpDecorate %[[LANE:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_RAYGEN-DAG: OpDecorate %[[LANE]] Volatile
// CHECK_RAYGEN-DAG: OpDecorate %[[SIZE:[A-Za-z0-9_]+]] BuiltIn SubgroupSize
// CHECK_RAYGEN-DAG: OpDecorate %[[SIZE]] Volatile
// CHECK_RAYGEN-DAG: OpGroupNonUniformElect
// CHECK_RAYGEN-DAG: OpGroupNonUniformBallot
// CHECK_RAYGEN-DAG: OpGroupNonUniformBroadcastFirst
[shader("raygeneration")]
void rayGenMain()
{
    bool isFirst = WaveIsFirstLane();
    uint laneIdx = WaveGetLaneIndex();
    uint laneCount = WaveGetLaneCount();
    uint4 ballot = WaveActiveBallot(laneIdx < 16);
    uint firstValue = WaveReadLaneFirst(laneIdx);
    output[0] = uint(isFirst) + laneIdx + laneCount + ballot.x + firstValue;
}

// CHECK_CLOSESTHIT-DAG: OpDecorate %[[V2:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_CLOSESTHIT-DAG: OpDecorate %[[V2]] Volatile
[shader("closesthit")]
void closestHitMain(inout Payload p, in Attribs a)
{
    p.v = WaveGetLaneIndex();
}

// CHECK_MISS-DAG: OpDecorate %[[V3:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_MISS-DAG: OpDecorate %[[V3]] Volatile
[shader("miss")]
void missMain(inout Payload p)
{
    p.v = WaveGetLaneIndex();
}

// CHECK_INTERSECTION-DAG: OpDecorate %[[V4:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_INTERSECTION-DAG: OpDecorate %[[V4]] Volatile
[shader("intersection")]
void intersectionMain()
{
    output[0] = WaveGetLaneIndex();
}

// CHECK_CALLABLE-DAG: OpDecorate %[[V5:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_CALLABLE-DAG: OpDecorate %[[V5]] Volatile
[shader("callable")]
void callableMain(inout CallableData c)
{
    c.v = WaveGetLaneIndex();
}

// Via-glsl raygen — pins glslang's `rtSubgroupDecls` `Volatile` emission.
// CHECK_VIA_GLSL-DAG: OpDecorate %[[LANE_VIA:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_VIA_GLSL-DAG: OpDecorate %[[LANE_VIA]] Volatile
