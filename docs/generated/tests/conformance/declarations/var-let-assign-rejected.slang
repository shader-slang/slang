//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=Assigning to a `let` variable is rejected by the compiler with E30011 (not an l-value).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A `let` declaration introduces an immutable variable, which
// may not be assigned to".  Assigning must be a compile error.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void main()
{
    let x = 5;
    x = 10;
}

// CHECK-DAG: error
// CHECK-DAG: E30011
// CHECK-DAG: l-value
