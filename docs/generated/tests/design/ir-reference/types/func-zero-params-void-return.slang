//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=021d788619e4517ac1c3910bae98363f85bfa77ed12c6b9a50b7bd8e1f64f627
//META: purpose=Boundary: a zero-parameter void function is typed Func(Void) with no parameter operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Func` note says the result type is the leading operand
// and the rest are parameter types. The minimal Func is one with
// no params and Void result; it appears as `Func(Void)` with no
// further operands.

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
