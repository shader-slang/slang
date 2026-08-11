//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=ebbc1fbf276a26d7c6caf4f51c9ba3ba143103b1107f9217e594e9044a25d6d2
//META: purpose=The precedence comparison that `maybeEmitParens` starts from keeps the parentheses an expression needs to preserve semantics: an addition used as a multiplication operand emits as `(a + b) * c` on every C-like target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the baseline half of the parenthesization rule. `+` binds looser
// than `*`, so dropping the parens would change the value; every backend must
// keep them. Operands come from the dispatch id and are each used twice, so
// they survive lowering as named locals and the emitted expression can be
// matched by name. The result is stored into a buffer so it is not eliminated.

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
    outBuf[0] = (a + b) * c;
    outBuf[1] = a - b - c;
}

// HLSL: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
// GLSL: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
// METAL: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
// WGSL: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
// CUDA: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
// CPP: = (a_{{[0-9]+}} + b_{{[0-9]+}}) * c_{{[0-9]+}};
