//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the automatic location assignment for ray payloads, callable payloads and hit-object attributes that were declared without an explicit location, which skips locations already claimed explicitly.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `getAllNullLocationRayObjectsAndUsedLocations` /
// `assignRayPayloadHitObjectAttributeLocations`
// (slang-ir-glsl-legalize.cpp:4512 and :4563). A payload declared without a
// location carries `-1` in its decoration; the pass makes two sweeps, first
// collecting the locations that were pinned explicitly, then handing out the
// lowest unused number to each `-1` in turn. The three kinds
// (RayPayloadKHR / CallableDataKHR / HitObjectAttributeEXT) each get their own
// independent counter, so the same number may legitimately be reused across
// kinds. This test declares one explicitly-located object of each kind and
// then leaves others automatic, so CHECK can assert both halves at once: the
// explicit numbers survive untouched, and each auto-assigned variable lands on
// a distinct number within its own kind that steps over the explicit one. The
// `TraceRay`/`CallShader` calls also introduce compiler-generated payload
// temporaries, which participate in the same numbering.
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry rayMain -stage raygeneration

struct Payload { float4 color; }
struct Attribs { float2 bary; }

[__vulkanRayPayload(1)] static Payload gExplicitPayload;
[__vulkanRayPayload] static Payload gAutoPayloadA;
[__vulkanRayPayload] static Payload gAutoPayloadB;
[__vulkanCallablePayload(0)] static Payload gExplicitCallable;
[__vulkanCallablePayload] static Payload gAutoCallable;
[__vulkanHitObjectAttributes(2)] static Attribs gExplicitAttrs = {float2(0)};
[__vulkanHitObjectAttributes] static Attribs gAutoAttrs = {float2(0)};

RaytracingAccelerationStructure gScene;
RWStructuredBuffer<float4> gOut;

[shader("raygeneration")]
void rayMain()
{
    RayDesc ray;
    ray.Origin = float3(0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    TraceRay(gScene, 0, 0xff, 0, 1, 0, ray, gAutoPayloadA);
    TraceRay(gScene, 0, 0xff, 0, 1, 0, ray, gAutoPayloadB);
    TraceRay(gScene, 0, 0xff, 0, 1, 0, ray, gExplicitPayload);
    CallShader(0, gAutoCallable);
    CallShader(1, gExplicitCallable);
    gOut[0] = gAutoPayloadA.color + gAutoPayloadB.color + gExplicitPayload.color +
              gAutoCallable.color + gExplicitCallable.color +
              float4(gExplicitAttrs.bary, gAutoAttrs.bary);
}

// Explicit locations are preserved verbatim.
//SPIRV-DAG: OpDecorate %gExplicitPayload Location 1
//SPIRV-DAG: OpDecorate %gExplicitCallable Location 0
//SPIRV-DAG: OpDecorate %gExplicitAttrs Location 2
// Auto-assigned ray payloads step over the explicitly claimed 1, and over
// location 0 which the TraceRay temporary took first.
//SPIRV-DAG: OpDecorate %gAutoPayloadA Location 2
//SPIRV-DAG: OpDecorate %gAutoPayloadB Location 3
// The callable-payload counter is independent of the ray-payload counter.
//SPIRV-DAG: OpDecorate %gAutoCallable Location 2
// The hit-object-attribute counter is independent again, so 0 is still free.
//SPIRV-DAG: OpDecorate %gAutoAttrs Location 0
// Each kind lands in its own storage class.
//SPIRV-DAG: %gAutoPayloadA = OpVariable %_ptr_RayPayloadKHR_Payload RayPayloadKHR
//SPIRV-DAG: %gAutoCallable = OpVariable %_ptr_CallableDataKHR_Payload CallableDataKHR
//SPIRV-DAG: %gAutoAttrs = OpVariable %_ptr_HitObjectAttributeEXT_Attribs HitObjectAttributeEXT
