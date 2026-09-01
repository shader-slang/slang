//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=09e8a26796107f4ea870ece7161025de2c11ac3fe6f559cd6a8ba18ea88a706d
//META: purpose=AddCtorOverloadCandidate contributes a type's constructor declarations as candidates, and the resolver picks among them on argument types like free functions.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constructor candidate family. `Pair` declares a one-argument and
// a two-argument __init; the call supplies two arguments. CHECK asserts the
// tag written by the two-argument constructor, which proves both constructors
// entered the candidate set and the arity/type filters picked the right one.

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
