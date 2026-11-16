//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=Assigning to a `let` variable is rejected by the compiler with E30011 (not an l-value).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that assigning to a `let` variable is rejected as not an l-value.
// `x = 10` after `let x = 5` must produce E30011; CHECK-DAG lines assert both
// the error severity, the code, and the "l-value" message fragment.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK, exit-code=-1):

void main()
{
    let x = 5;
    x = 10;
}

// CHECK-DAG: error
// CHECK-DAG: E30011
// CHECK-DAG: l-value
