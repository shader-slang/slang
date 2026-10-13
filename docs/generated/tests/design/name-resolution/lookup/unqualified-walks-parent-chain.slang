//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=ce74e751abd83caf10326ac968c3af12829a5d0d24575d5595ca7f1148931ff5
//META: purpose=The unqualified-lookup outer loop walks the scope parent chain outward to the module root, so a name inside a nested block resolves through the enclosing function scope and then the file scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that unqualified lookup keeps stepping to the parent scope
// until it finds the name. `p` is two scopes out (block -> function
// parameter scope) and `K` is three scopes out (block -> function ->
// file). CHECK asserts the sum 7 + 1000, which is only produced when
// both names resolved through the outward walk.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 1000;

int adder(int p)
{
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
