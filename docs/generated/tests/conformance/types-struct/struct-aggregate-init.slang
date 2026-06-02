//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=Without a user-provided constructor, aggregate initialization fills non-static fields in order from the initializer list; extra unspecified fields are default-initialized.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "When no user-provided constructor is defined, an aggregate initialization is performed.
// An initializer list contains values for the struct non-static data members.
// If the initializer list does not contain enough values, the remaining data members are default-initialized."

//TEST:INTERPRET(filecheck=CHECK):

struct Plain
{
    int x;
    int y;
    int z;
}

void main()
{
    // Aggregate with full list
    Plain p1 = { 10, 20, 30 };
    // CHECK: 10 20 30
    printf("%d %d %d\n", p1.x, p1.y, p1.z);

    // Aggregate with partial list; z gets default initialized
    Plain p2 = { 5, 6 };
    // CHECK: 5 6
    printf("%d %d\n", p2.x, p2.y);
}
