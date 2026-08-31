//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=ac5209f47969a57048460bb2059320973fc1da4f9fbf7670b21dda1072978507
//META: purpose=A shift whose operand is an addition, `a << b + c`, gains parentheses around the addition even though C precedence already evaluates it first, because shift combinations are one of the sets the emitter deliberately over-parenthesizes.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shift arm of the deliberate over-parenthesization rule, and
// contrasts with the arithmetic-only case: `a + b * c` stays bare while
// `a << b + c` does not. Both expressions are emitted from the same entry
// point so a single run shows the rule is operator-set-specific and not a
// blanket "parenthesize everything". Operands come from the dispatch id so
// nothing is constant-folded away.

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
    outBuf[0] = a << b + c;
    outBuf[1] = a + b * c;
}

// HLSL: a_{{[0-9]+}} << (b_{{[0-9]+}} + c_{{[0-9]+}})
// GLSL: a_{{[0-9]+}} << (b_{{[0-9]+}} + c_{{[0-9]+}})
// METAL: a_{{[0-9]+}} << uint(b_{{[0-9]+}} + c_{{[0-9]+}})
// WGSL: a_{{[0-9]+}} << {{[(]+}}u32(b_{{[0-9]+}} + c_{{[0-9]+}})
// CUDA: a_{{[0-9]+}} << (b_{{[0-9]+}} + c_{{[0-9]+}})
// CPP: a_{{[0-9]+}} << (b_{{[0-9]+}} + c_{{[0-9]+}})
