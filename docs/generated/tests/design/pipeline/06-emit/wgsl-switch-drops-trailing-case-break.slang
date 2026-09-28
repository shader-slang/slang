//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#wgsl
//META: doc_section_digest=24cf127ba87a077887a4ad8e725f5ae07b1714ceed5cc4f322ad01d02f3bfd6e
//META: purpose=WGSL sets `shouldEmitSwitchCaseTerminatingBreak` to false, so the `break` Slang places at the tail of every switch case is omitted from WGSL output while the other C-like backends still emit it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WGSL-only suppression of the redundant trailing break. WGSL
// case bodies never fall through, and older validators reject a `break`
// outside a loop, so the tail break must not be printed. Every case in the
// switch below ends in `break`; the WGSL prefix asserts none survives
// (CHECK-NOT over the whole output), while HLSL, GLSL and Metal assert theirs
// do — so the test would also fail if the suppression leaked into a shared
// path. GLSL is the interesting third case because it is the one non-WGSL
// backend whose switch selector is spelled from a builtin rather than a
// parameter. SPIR-V has no textual `break` — a switch lowers to `OpSwitch`
// plus branches — so it is recorded in the README's Untested-claims row.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    switch (int(tid.x))
    {
    case 0:
        r = 10;
        break;
    case 1:
        r = 20;
        break;
    default:
        r = 30;
        break;
    }
    outBuf[0] = r;
}

// WGSL: switch(i32(tid_{{[0-9]+}}.x))
// WGSL-NOT: break;

// HLSL: switch(int(tid_{{[0-9]+}}.x))
// HLSL: break;

// METAL: switch(int(tid_{{[0-9]+}}.x))
// METAL: break;

// GLSL: switch(int(gl_GlobalInvocationID.x))
// GLSL: break;
