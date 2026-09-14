//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#unreachable-code-diagnostics-come-from-lowering
//META: doc_section_digest=f9c18ab4f45129a1130db9cf3d740296e5fcae9baedfabc1f8bafac13f8656c6
//META: purpose=A statement written after a return is diagnosed as unreachable code by the lowering pass, not by the semantic checker.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unreachable-code diagnostic that the doc places in lowering
// rather than in the checker: once a block has a terminator, the next
// statement has no label to be branched to, so lowering reports it. The
// statement after `return;` is well-typed on its own, so a checker-only
// pipeline would accept it silently; the CHECK pins the diagnostic code and
// caret span on the offending statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = n;
    return;
    buf[1] = n + 1;
//CHECK: E41000
}
