//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=dc101149ff9b445d781bc38dc82164f3a5d08e36d3931fcf7594b780eca06062
//META: purpose=`NewExpr` parses as `'new' Type ('(' ArgList? ')')?` and yields an object whose members are then assignable.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the NewExpr atom in its empty-argument-list form. `slangi` cannot
// instantiate a `class`, so the observation is taken on emitted HLSL instead.
// The object is written to and then read back into the output buffer, which
// keeps the value from being dead-code-eliminated; the store of 21 reaching
// the buffer proves `new Box()` produced a real object rather than being
// parsed as something inert.
//
// The `class` declaration doubles as the ClassDecl production's positive
// case: NewExpr's operand has to be a reference type for the form to apply.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

class Box
{
    int v;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Box b = new Box();
    b.v = 21;
    outputBuffer[0] = b.v;
}

// CHECK: outputBuffer_{{[0-9]+}}[int(0)] = int(21);
