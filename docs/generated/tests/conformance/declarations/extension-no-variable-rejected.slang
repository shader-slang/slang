//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/language-reference/declarations.md#extensions
//META: doc_section_digest=ec41e5f3bc9673746ac30b0bb1f19e54c7c9d47f0377ab73cc170b70580c168d
//META: purpose=The body of an extension may not include variable declarations; attempting to add a non-static member variable in an extension is a compile error (E31400).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that adding a non-static member variable inside an extension is rejected.
// Extensions cannot change a type's in-memory layout; declaring `extraField` inside
// `extension MyVec` must trigger E31400, confirmed by CHECK-DAG.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK, exit-code=-1):

struct MyVec { float x, y; }

extension MyVec
{
    int extraField;
}

void main() {}

// CHECK-DAG: error
// CHECK-DAG: E31400
