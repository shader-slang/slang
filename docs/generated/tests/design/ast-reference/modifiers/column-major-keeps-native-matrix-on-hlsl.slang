//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#matrix-layout-modifiers
//META: doc_section_digest=f76a647549de7fb5c4cc6954081de333e1fe1179684bb921f24c16acba6f921e
//META: purpose=`column_major` on a constant-buffer matrix keeps the native matrix storage in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the paired `column_major` spelling, which names Slang's native
// convention, so no storage rewrite is needed. CHECK asserts the emitted field is
// still a plain `float4x4` and CHECK-NOT asserts the row-array storage struct
// that `row_major` forces is absent -- together they show the two modifiers
// select genuinely different layouts rather than both being ignored.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params
{
    column_major float4x4 m;
};

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = m[0][1];
}

//CHECK: float4x4 m
//CHECK-NOT: _MatrixStorage
