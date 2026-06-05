//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=Writing to a non-static data member inside a non-`[mutating]` member function is rejected by the compiler.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that writing to a non-static member field in a non-`[mutating]` function is rejected.
// badWrite() assigns to `a` without the attribute; CHECK-DAG asserts E30011 (l-value) fires.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct TestStruct
{
    int a;

    int badWrite()
    {
        a = 5;
        return a;
    }
}

void main()
{
}

// CHECK-DAG: error
// CHECK-DAG: E30011
// CHECK-DAG: l-value
