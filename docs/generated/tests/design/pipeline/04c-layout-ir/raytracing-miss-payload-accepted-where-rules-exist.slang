//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#diagnosed-failures-during-binding
//META: doc_section_digest=210845004e453a7583de469b0de24d48d58f75a96ce4c3d08e88229e3bb5eb0d
//META: purpose=A family that supplies ray-payload rules binds the miss payload instead of diagnosing it, so the same shader that fails on Metal compiles for SPIR-V and for CUDA.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accepting half of the missing-rules claim, which is what
// makes the diagnostic a *target* limitation rather than a stage
// limitation. The same `miss` entry point that draws E39032 on Metal and
// C++ is compiled here for two families documented as supplying the
// ray-payload accessor. The SPIR-V CHECK asserts the payload was actually
// given a ray-payload storage class (so binding ran and produced a
// placement), and the CUDA CHECK asserts the payload reached the OptiX
// payload-register accessors, which is only emitted once the parameter has
// a layout.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage miss
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage miss

struct Payload
{
    float4 color;
}

[shader("miss")]
void main(inout Payload payload)
{
    payload.color = float4(1, 0, 0, 1);
}

// SPV-DAG: OpEntryPoint MissKHR
// SPV-DAG: %{{[A-Za-z0-9_]+}} = OpVariable %{{[A-Za-z0-9_]+}} IncomingRayPayloadKHR

// CUDA-DAG: __miss__main
// CUDA-DAG: optixGetPayload_0()
