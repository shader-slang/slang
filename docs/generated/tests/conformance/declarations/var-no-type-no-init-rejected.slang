//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=It is an error to declare a variable with neither a type specifier nor an initial-value expression; compiler emits E30620.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `var x;` with neither a type nor an initializer is a compile error.
// The compiler must emit E30620; CHECK-DAG lines assert the error severity and code.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void main()
{
    var x;
}

// CHECK-DAG: error
// CHECK-DAG: E30620
