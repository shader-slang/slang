//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=61c0250465db1de2183c1e86044b9101be702b2abc84656a2bce95e208e63cb5
//META: purpose=AddCtorOverloadCandidate produces candidates from a type's ConstructorDecls; the overload resolver picks among constructors based on argument types just like free functions.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two constructors of `Pair`: one with two ints, one with one int.
// Passing two ints picks the two-arg constructor.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair
{
    int tag;
    int a;
    int b;

    __init(int x, int y) { tag = 2; a = x; b = y; }
    __init(int x)        { tag = 1; a = x; b = 0; }
}

void main()
{
    Pair p = Pair(3, 4);
    // CHECK: tag=2 a=3 b=4
    printf("tag=%d a=%d b=%d\n", p.tag, p.a, p.b);
}
