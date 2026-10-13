//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Expansion: a user-defined helper function emits as a `RetType name_<N>(args) { ... }` declaration in HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL emit writes user-defined functions with the standard
// C-style signature; the function is name-mangled with the `_<N>`
// suffix and the call site references the mangled name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

int helper(int a, int b)
{
    return a * 2 + b;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = helper(int(tid.x), int(tid.y));
}

// CHECK: int helper_{{[0-9]+}}(
// CHECK: helper_{{[0-9]+}}(
