//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=A `switch` on a `bool` selector is rewritten into an integer switch for GLSL, SPIR-V, and WGSL, while HLSL keeps the boolean selector.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bool-switch rewrite the GLSL-legalize row names for the
// three targets that require an integer switch selector. The selector is
// derived from SV_DispatchThreadID so it stays runtime-varying and the
// switch cannot be constant-folded away. On GLSL the CHECK asserts the
// selector is wrapped in `int(...)` and the case label is `1`; on SPIR-V
// the `OpSwitch` literal is `1`; on WGSL the selector is widened with
// `select(i32(0), i32(1), ...)` and the case label is `i32(1)`. HLSL is
// the contrast target: it is not in the rewrite list, so `case true:`
// survives verbatim.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool selector = tid.x > 0;
    int r = 0;
    switch (selector)
    {
    case true:
        r = 11;
        break;
    default:
        r = 22;
        break;
    }
    buf[0] = r;
}

// The GLSL selector is coerced to an integer and the `true` label becomes 1.
// GLSL-DAG: switch(int(
// GLSL-DAG: case 1:

// SPIR-V carries the integer literal 1 as the OpSwitch case value.
// SPIRV: OpSwitch %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} 1 %{{[A-Za-z0-9_]+}}

// WGSL widens the bool to i32 through `select` and labels the case i32(1).
// WGSL-DAG: switch(select(i32(0), i32(1),
// WGSL-DAG: case i32(1):

// HLSL is not in the rewrite list, so the boolean case label survives.
// HLSL: case true:
