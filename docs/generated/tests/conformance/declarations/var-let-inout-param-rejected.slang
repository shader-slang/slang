//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A `let` variable may not be used as argument for an `inout` parameter; the compiler rejects it with E30047.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `let` variable cannot be passed as an `inout` argument.
// Passing the immutable `x` to `add1(inout int v)` must trigger E30047; CHECK-DAG
// asserts the error severity and code.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void add1(inout int v) { v += 1; }

void main()
{
    let x = 5;
    add1(x);
}

// CHECK-DAG: error
// CHECK-DAG: E30047
