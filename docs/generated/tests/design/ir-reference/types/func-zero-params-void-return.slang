//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=fdd06f0a3a11b54325a8474139094f83e3f6174898d99c25f5f4cdcfc864d79a
//META: purpose=Boundary: a zero-parameter void function is typed Func(Void) with no parameter operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a zero-parameter void function is typed Func(Void) with no parameter
// operands. The minimal Func is one with no params and Void result; it
// appears as `Func(Void)` with no further operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

void doNothing() {}

[numthreads(1, 1, 1)]
void main()
{
    doNothing();
    buf[0] = a;
}

// IR-LABEL: func %doNothing
// IR-SAME: Func(Void)
