//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=43e51aa8f8900cb25c2253ee8c4a700f294af52f3a04c9121c8e46e771ee0e72
//META: purpose=When the scope walk reaches an enclosing aggregate with a receiver available, _lookUpInScopes rewrites the request into member lookup and records ThisValue even from nested blocks.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference to `x` sits two nested blocks inside the method. `_lookUpInScopes` crosses both
// block scopes before it reaches the struct, rewrites the request as member lookup, and records a
// structural `ThisValue` step. `ConstructLookupResultExpr` replays that step, so `x` binds to the
// receiver's field.

//TEST:INTERPRET(filecheck=CHECK):

struct P
{
    int x;
    int complicated(int n)
    {
        {
            {
                return x + n;
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
