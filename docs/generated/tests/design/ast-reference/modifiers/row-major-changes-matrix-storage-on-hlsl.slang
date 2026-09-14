//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#matrix-layout-modifiers
//META: doc_section_digest=fe0e81537eb72b7c28145e1bc1dd83b677979f956278af7614c7e73e4b4bf96f
//META: purpose=`row_major` on a constant-buffer matrix changes its emitted storage shape away from the native matrix type.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `row_major` actually reaches layout rather than parsing and being
// dropped. Slang's native matrix convention is column-major, so a `row_major`
// field cannot be stored as a plain `float4x4`; the compiler instead emits a
// dedicated row-array storage struct for it. CHECK asserts that storage struct
// appears, which happens only when the layout modifier took effect.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params
{
    row_major float4x4 m;
};

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = m[0][1];
}

//CHECK: _MatrixStorage_float4x4{{[A-Za-z0-9_]*}}
