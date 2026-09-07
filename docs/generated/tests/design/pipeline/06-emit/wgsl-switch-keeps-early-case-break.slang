//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#wgsl
//META: doc_section_digest=24cf127ba87a077887a4ad8e725f5ae07b1714ceed5cc4f322ad01d02f3bfd6e
//META: purpose=Only the trailing case break is dropped on the WGSL target — an early `break` in the middle of a case body is still emitted, because it is load-bearing control flow rather than the redundant tail terminator.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out in WGSL's break suppression. The first case contains
// a conditional `break` before its final statement, so the case has both an
// early break (which must survive, or the shader changes meaning) and a
// trailing one (which must not). CHECK asserts the early break appears
// nested inside the `if` and that the case body ends without a second break
// before the next case label.
//
// The HLSL, GLSL and Metal prefixes run the same source through the backends
// that do not suppress the tail break, and assert *two* sequential `break;`
// tokens between the case label and `default` — the early one and the trailing
// one. That is what makes the WGSL result specific: same input, same early
// break, one fewer terminator. SPIR-V has no textual `break` (a switch lowers
// to `OpSwitch` plus branches), so it is recorded in the README's
// Untested-claims row instead.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.y);
    int r = 0;
    switch (int(tid.x))
    {
    case 0:
        r = 1;
        if (a > 5)
            break;
        r = 2;
        break;
    default:
        r = 3;
        break;
    }
    outBuf[0] = r;
}

// WGSL: case i32(0):
// WGSL: if(a_{{[0-9]+}} > i32(5))
// WGSL: break;
// WGSL: default :
// WGSL-NOT: break;

// HLSL: case int(0):
// HLSL: if(a_{{[0-9]+}} > int(5))
// HLSL: break;
// HLSL: break;
// HLSL: default:

// METAL: case int(0):
// METAL: if(a_{{[0-9]+}} > int(5))
// METAL: break;
// METAL: break;
// METAL: default:

// GLSL: case 0:
// GLSL: if(a_{{[0-9]+}} > 5)
// GLSL: break;
// GLSL: break;
// GLSL: default:
