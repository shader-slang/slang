//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=ac5209f47969a57048460bb2059320973fc1da4f9fbf7670b21dda1072978507
//META: purpose=Emitted code is precedence-aware rather than minimally parenthesized: `a | b & c` gains parentheses around the `&` operand even though C precedence already binds it first, because downstream compilers warn about mixed bitwise operators.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the deliberate over-parenthesization the emitter applies to
// bitwise combinations. `&` already binds tighter than `|`, so a purely
// minimal printer would emit `a | b & c`; CHECK asserts the emitted form is
// `a | (b & c)` on every C-like target, which is only produced if the extra
// parens are forced. Operands are read from the dispatch id and used twice so
// they survive as named locals, and the result is stored into a buffer.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = int(tid.y);
    int c = int(tid.z);
    outBuf[0] = a | b & c;
    outBuf[1] = a - b - c;
}

// HLSL: a_{{[0-9]+}} | (b_{{[0-9]+}} & c_{{[0-9]+}})
// GLSL: a_{{[0-9]+}} | (b_{{[0-9]+}} & c_{{[0-9]+}})
// METAL: a_{{[0-9]+}} | (b_{{[0-9]+}} & c_{{[0-9]+}})
// WGSL: a_{{[0-9]+}} | {{[(]+}}b_{{[0-9]+}} & {{[(]*}}c_{{[0-9]+}}
// CUDA: a_{{[0-9]+}} | (b_{{[0-9]+}} & c_{{[0-9]+}})
// CPP: a_{{[0-9]+}} | (b_{{[0-9]+}} & c_{{[0-9]+}})
