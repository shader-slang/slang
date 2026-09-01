//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a constant buffer whose struct ends in an unsized ordinary array is rejected with E31215 naming the trailing field.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: a constant buffer whose element type ends in an unsized
// array of an ordinary (non-resource) element cannot be laid out for any target,
// so `checkVarDeclCommon` walks the element type with
// `getTrailingUnsizedArrayElement` and reports the offending field. The
// annotations pin the observed E31215 on the field plus the follow-up note that
// points back at the constant-buffer declaration that forced the walk.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

struct Inner
{
        int header;
        int payload[];
//CHECK:    ^^^^^^^ E31215
//CHECK:    ^^^^^^^ cannot use unsized type 'int[]' in a constant buffer.
}

ConstantBuffer<Inner> cb;
//CHECK:              ^^ see constant buffer definition.

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}
