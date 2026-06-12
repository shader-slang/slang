//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Negative probe of the documented arithmetic family: `+` is only defined for the numeric / arithmetic-interface types the doc's `add` opcode applies to; applying it to a user `struct` with no `+` overload fails type checking before IR lowering.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `add` opcode applies to the arithmetic-interface family
// (integers / floats / vectors / matrices) — adding two raw `struct`
// values is therefore not a valid lowering input and the front end
// rejects it before any IR is built. This pins down the absence of an
// implicit struct-wise `add`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry main

struct Foo { int x; }
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Foo a;
    Foo b;
    buf[0] = (a + b).x;
//CHECK:        ^ no overload for '+' applicable to arguments of type (Foo, Foo)
}
