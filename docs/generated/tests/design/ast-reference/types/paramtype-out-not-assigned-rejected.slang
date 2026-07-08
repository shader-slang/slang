//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=An OutParamType requires the callee to assign before read; reading an out parameter before assigning it is diagnosed as use of an uninitialized value.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of OutParamType semantics: an out parameter starts
// unassigned, so reading it before writing is a use of an
// uninitialized value. CHECK pins the uninitialized-use diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void readsOut(out int x)
{
    int y = x + 1;
//CHECK:      ^ use of uninitialized out parameter 'x'
    x = y;
}

void main() { int z; readsOut(z); }
