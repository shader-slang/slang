//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=3b394bf839041f0d86487cb14f22d3c9442c777b87a2bbf98421edb81222a98a
//META: purpose=Step 5 of the unqualified-lookup walk updates ThisParameterMode based on the enclosing function's modifiers; [mutating] yields a mutable This breadcrumb so unqualified writes to a field in the method body update the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Step 5 of unqualified-lookup: "the loop updates
// `thisParameterMode` based on the container's modifiers —
// `[mutating]`, `[ref]`, `static`, and `__init` change the kind of
// `this` that any breadcrumb recorded in an outer scope must use".
// A `[mutating]` method's unqualified write to `count` must mutate
// the receiver — proving the This breadcrumb is recorded with the
// mutable mode.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int count;
    [mutating] void bump() { count = count + 1; }
}

void main()
{
    Counter c;
    c.count = 10;
    c.bump();
    c.bump();
    // CHECK: r=12
    printf("r=%d\n", c.count);
}
