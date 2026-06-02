//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=`const` data members cannot be initialized by the constructor; attempting to assign to a `const` field inside `__init` is a compiler error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "`const` data members cannot be initialized by the constructor."

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct HasConst
{
    const int c = 5;

    __init(int v)
    {
        c = v;
    }
}

void main()
{
}

// CHECK-DAG: error
// CHECK-DAG: E30011
// CHECK-DAG: l-value
