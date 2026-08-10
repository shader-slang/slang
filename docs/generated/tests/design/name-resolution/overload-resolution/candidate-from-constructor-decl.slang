//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=dfd7b5878428e66d4c6b22b8ce6a609a70c3ed9c74c32c1f56d5d54a8a3e67d5
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
