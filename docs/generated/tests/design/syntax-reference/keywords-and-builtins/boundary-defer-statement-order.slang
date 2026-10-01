//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=0d8e1ca053b429da8e96d222f706189d5ed5e28ff7285b41aa4ffea890ca7d1f
//META: purpose=Boundary: `defer` is a statement keyword whose body is held back to scope exit, so its output lands after the statements that textually follow it.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `defer` is parsed as its own statement keyword and that
// its body is deferred rather than executed in place. The deferred
// `printf` is written first but must print last; CHECK asserts the
// single line "before after", which is only produced if the body ran
// at scope exit.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    defer printf("after");
    printf("before ");
    // CHECK: before after
}
