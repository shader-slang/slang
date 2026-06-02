//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Multiple conformance constraints on a single type parameter (inline `T : A` plus `where T : B`) are all enforced; operations from both interfaces are available in the function body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states conformance constraints can be expressed inline (T : IFace) or
// via `where` clauses. Multiple constraints are cumulative.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasA { int getA(); }
interface IHasB { int getB(); }

struct AB : IHasA, IHasB
{
    int getA() { return 3; }
    int getB() { return 5; }
}

func sumAB<T : IHasA>(T x) -> int where T : IHasB
{
    return x.getA() + x.getB();
}

void main()
{
    AB obj;
    // CHECK: 8
    printf("%d\n", sumAB(obj));
}
