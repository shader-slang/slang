//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=ac5209f47969a57048460bb2059320973fc1da4f9fbf7670b21dda1072978507
//META: purpose=Relational operands of an equality comparison are parenthesized on emit — `a < b == c < d` comes out as `(a < b) == (c < d)` — even though C precedence already groups them that way.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the relational/equality arm of the deliberate over-parenthesization
// rule. `<` binds tighter than `==`, so the parentheses are semantically
// redundant; they are emitted anyway because downstream compilers warn about
// the unparenthesised form. The comparison feeds an `if` so the boolean
// escapes into a stored value and cannot be folded, and the four operands come
// from the dispatch id.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupID)
{
    int a = int(tid.x);
    int b = int(tid.y);
    int c = int(gid.x);
    int d = int(gid.y);
    outBuf[0] = (a < b == c < d) ? 1 : 0;
    outBuf[1] = a + b + c + d;
}

// HLSL: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
// GLSL: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
// METAL: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
// WGSL: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
// CUDA: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
// CPP: (a_{{[0-9]+}} < b_{{[0-9]+}}) == (c_{{[0-9]+}} < d_{{[0-9]+}})
