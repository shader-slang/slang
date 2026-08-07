//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=a59d860b5f6a346b23bed14b56396e2c1ea41a3005067b975c01053cca1644c4
//META: purpose=The shared C-like base walks the IR and emits one declaration at a time, so a user struct with heterogeneous fields becomes a `struct` declaration ahead of its first use on every textual backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the declaration-ordering behaviour that all C-like backends
// inherit from the shared base: the struct type is emitted before the
// function that takes it. Sequential (not -DAG) CHECKs are used precisely
// because ordering is the property under test. The struct is passed to a
// helper so it survives scalarization, and the helper's result escapes into a
// buffer so nothing is dead-code eliminated. WGSL is included because it is a
// C-like-base subclass even though its declaration syntax differs.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct Payload
{
    int   a;
    float b;
    uint  c;
};

RWStructuredBuffer<float> outBuf;

float useIt(Payload p)
{
    return float(p.a) + p.b + float(p.c);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Payload p;
    p.a = int(tid.x);
    p.b = float(tid.y);
    p.c = tid.z;
    outBuf[0] = useIt(p);
}

// HLSL: struct Payload_{{[0-9]+}}
// HLSL: float useIt_{{[0-9]+}}(Payload_

// GLSL: struct Payload_{{[0-9]+}}
// GLSL: float useIt_{{[0-9]+}}(Payload_

// METAL: struct Payload_{{[0-9]+}}
// METAL: float useIt_{{[0-9]+}}(

// WGSL: struct Payload_{{[0-9]+}}
// WGSL: fn useIt_{{[0-9]+}}(

// CUDA: struct Payload_{{[0-9]+}}
// CUDA: __device__ float useIt_{{[0-9]+}}(

// CPP: struct Payload_{{[0-9]+}}
// CPP: static float useIt_{{[0-9]+}}(
