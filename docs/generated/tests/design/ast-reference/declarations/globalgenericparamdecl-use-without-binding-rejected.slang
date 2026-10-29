//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#globalgenericparamdecl-and-globalgenericvalueparamdecl
//META: doc_section_digest=e16c4a24162c83091e75ad0034be9aa82cec2296d141344574cdaee52215d332
//META: purpose=A module-level type_param is a specialization parameter of the module whose binding arrives from outside, so generating target code from a use of it without a binding reports E38207 at the declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies where the binding for a module-scope generic parameter has to come
// from. `type_param T : IProducer;` takes no argument where it is written, so
// nothing inside the module can supply one; the parameter is instead collected
// as a specialization parameter of the module itself. That is why parsing and
// checking the declaration alone succeed and only *generating code from a use*
// fails. CHECK pins E38207 and, through the caret, the fact that the error is
// reported at the declaration rather than at the use site - which is what
// tells a reader the declaration is the thing missing an argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry computeMain

interface IProducer
{
    int produce();
}

type_param T : IProducer;
//CHECK:   ^ E38207

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    T v;
    outputBuffer[0] = v.produce();
}
