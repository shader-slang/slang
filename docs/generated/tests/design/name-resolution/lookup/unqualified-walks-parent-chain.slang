//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=3b394bf839041f0d86487cb14f22d3c9442c777b87a2bbf98421edb81222a98a
//META: purpose=The unqualified-lookup outer loop walks request.scope->parent chain to the module root, so an inner block resolves a function-scope parameter through the enclosing function scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 1 of the algorithm: "Iterate over scopes ... walks
// request.scope to request.endScope via the parent chain". A name
// referenced inside a nested block must be resolved by walking
// outward through the function scope, then the file scope.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 1000;

int adder(int p)
{
    // Two parent steps away: the local block walks out to the
    // function parameter scope and again out to the file scope to
    // reach `K`.
    {
        int sum = p + K;
        return sum;
    }
}

void main()
{
    // CHECK: r=1007
    printf("r=%d\n", adder(7));
}
