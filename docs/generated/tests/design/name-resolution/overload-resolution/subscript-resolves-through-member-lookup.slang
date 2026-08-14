//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=fcdc0ef5f61233fa8b4fb6def9878dcee51b42c2d1bbde085f283c94b50f6fdd
//META: purpose=__subscript is resolved by member lookup on the indexed value's type, so a[i] dispatches to the type's subscript getter with the implicit this.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second member-lookup operator form. Indexing a struct value
// resolves __subscript on its type and replays the this breadcrumb, so the
// getter can read the receiver's field; CHECK asserts a result that combines
// the stored base with the index.

//TEST:INTERPRET(filecheck=CHECK):

struct Grid
{
    int base;
    __subscript(int i) -> int { get { return base + i; } }
}

void main()
{
    Grid g;
    g.base = 100;
    int tag = g[7];
    // CHECK: tag=107
    printf("tag=%d\n", tag);
}
