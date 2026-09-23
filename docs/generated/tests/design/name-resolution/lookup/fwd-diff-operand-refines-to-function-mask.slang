//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=991fc4058be6fd34aec1a4296d60bb722bc2f19a97c9a91c4e3f65165643bfd2
//META: purpose=The operand of fwd_diff is resolved against a function-only mask, so a same-named non-function candidate accumulated from an outer scope is dropped silently rather than reported as ambiguous.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the narrow-mask re-filter. `scale` names a struct at file
// scope and a differentiable function inside the namespace. Functions
// are overloadable, so the outward scope walk does not stop at the
// namespace hit and the file-scope type accumulates alongside it,
// leaving two items of different categories. The fwd_diff operand is
// the use site that asks for the function category only, so the type
// is dropped without a diagnostic. CHECK asserts the derivative pair
// of x*x at x=3 (value 9, derivative 6) comes back and, implicitly,
// that no ambiguity error was raised — the compile would have failed
// instead of printing.

//TEST:INTERPRET(filecheck=CHECK):

struct scale { int q; }

namespace N
{
    [Differentiable]
    float scale(float x) { return x * x; }

    void run()
    {
        DifferentialPair<float> p = diffPair(3.0f, 1.0f);
        DifferentialPair<float> r = fwd_diff(scale)(p);
        // CHECK: v=9 d=6
        printf("v=%d d=%d\n", int(r.p), int(r.d));
    }
}

void main()
{
    N.run();
}
