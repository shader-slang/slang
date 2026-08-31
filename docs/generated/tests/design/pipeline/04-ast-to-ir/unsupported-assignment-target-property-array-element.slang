//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#diagnostics-during-lowering
//META: doc_section_digest=90444c4cb9fc02bd4dc517a83c3f36f04807193346a8cbe99ca628ff372a49b8
//META: purpose=Indexing an array-valued `property` in assignment position reaches the assignment-lowering switch with a left-hand side it cannot encode and reports `error[E40017]: assignment target is not supported` rather than aborting.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the reachable case of the assignment-lowering fallback. `s.v` is
// backed by a get/set accessor pair, so reading it materializes a `call` to the
// getter — a temporary r-value — and the trailing `[0]` then indexes that
// temporary. There is no storage for the write to land in, so lowering has no
// encodable left-hand side and diagnoses instead of crashing. The caret is on
// the `=` token, which is where the compiler attributes the assignment, and
// E40017 is the code that pins this specific fallback arm (a checker-level
// rejection would have fired earlier with a different code).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -stage compute -entry main

struct S
{
    float _v[2];
    property float v[2] { get { return _v; } set { _v = newValue; } }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s.v[0] = 1.0;
//CHECK:   ^ E40017
    outputBuffer[0] = s.v[0];
}
