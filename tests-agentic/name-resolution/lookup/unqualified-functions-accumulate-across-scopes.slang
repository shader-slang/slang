//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=3b394bf839041f0d86487cb14f22d3c9442c777b87a2bbf98421edb81222a98a
//META: purpose=Functions are overloadable, so step 6 does NOT short-circuit on a function hit; the lookup walks outward and a same-name function in an enclosing namespace remains a candidate.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 6 of unqualified lookup says functions and generics keep
// walking outward across scopes. Two `g` overloads — one in the
// enclosing namespace, one at file scope — must both be candidates
// at the call site inside the namespace.

//TEST:INTERPRET(filecheck=CHECK):

int g(int x) { return x + 1; }

namespace N
{
    int g(float x) { return int(x) + 100; }

    int call()
    {
        // g(3) selects the file-scope int overload; g(2.5f) selects
        // the namespace-local float overload. Both were accumulated.
        return g(3) + g(2.5f);
    }
}

void main()
{
    // CHECK: r=106
    printf("r=%d\n", N::call());
}
