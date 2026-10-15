//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=ce74e751abd83caf10326ac968c3af12829a5d0d24575d5595ca7f1148931ff5
//META: purpose=Callables are overloadable, so the outward scope walk does not short-circuit on a function hit and a same-named function in an enclosing scope stays a candidate.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the short-circuit rule exempts overloadable decls. The
// call site sits inside namespace N, which declares g(float); the file
// scope declares g(int). CHECK asserts 4 + 102, which requires the int
// overload from the *outer* scope to still be a candidate after the
// namespace-local float overload was already found.

//TEST:INTERPRET(filecheck=CHECK):

int g(int x) { return x + 1; }

namespace N
{
    int g(float x) { return int(x) + 100; }

    int call()
    {
        return g(3) + g(2.5f);
    }
}

void main()
{
    // CHECK: r=106
    printf("r=%d\n", N::call());
}
