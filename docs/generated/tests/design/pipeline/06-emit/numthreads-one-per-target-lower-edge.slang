//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=b39e80b2c14d1286101fea456d02c0ad8020250e1022fdc6eccd5a974bcbef9d
//META: purpose=Boundary: the smallest workgroup `[numthreads(1, 1, 1)]` is still emitted explicitly by every backend rather than collapsed or omitted.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the workgroup-size axis. A `1x1x1` workgroup is
// the identity case a backend could plausibly treat as "no attribute needed",
// so CHECK asserts the attribute is present and spelled with all three
// dimensions on every backend that has one. WGSL and GLSL spell it as a
// layout/attribute, SPIR-V as an execution mode, and the C++ backend as a
// comment above the dispatch shim, so each prefix pins its own form. Metal is
// not exercised here: MSL carries no source-level workgroup size at all.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

// HLSL: {{^}}[numthreads(1, 1, 1)]
// GLSL: layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
// SPIRV: OpExecutionMode %main LocalSize 1 1 1
// WGSL: {{^}}@workgroup_size(1, 1, 1)
// CPP: {{^}}// [numthreads(1, 1, 1)]
