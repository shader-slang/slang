//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=3b394bf839041f0d86487cb14f22d3c9442c777b87a2bbf98421edb81222a98a
//META: purpose=Step 4 of unqualified-lookup detects AggTypeDeclBase and rewrites the request to member lookup with a This breadcrumb; this rewrite still applies when the use site is several block scopes deep inside the method body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 4: "If the `containerDecl` is an `AggTypeDeclBase` ... the
// request is rewritten to perform member lookup against the
// corresponding `Type*`, with a `Breadcrumb::Kind::This` breadcrumb
// that records the implicit `this`/`This` of the enclosing decl."
// Verify the rewrite still fires when the reference is buried in
// nested blocks — the parent-chain walk traverses both blocks
// before reaching the struct.

//TEST:INTERPRET(filecheck=CHECK):

struct P
{
    int x;
    int complicated(int n)
    {
        {
            {
                return x + n;  // this.x via the AggType rewrite
            }
        }
    }
}

void main()
{
    P p;
    p.x = 50;
    // CHECK: r=53
    printf("r=%d\n", p.complicated(3));
}
