//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=dfd7b5878428e66d4c6b22b8ce6a609a70c3ed9c74c32c1f56d5d54a8a3e67d5
//META: purpose=AddDeclRefOverloadCandidates dispatches through a typedef to the aggregate type it names, so calling the alias enumerates the underlying type's constructors.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the typedef arm of candidate construction: the callee names an
// alias, not the struct, and the resolver still reaches the struct's
// constructors. CHECK asserts the fields the two-argument constructor wrote.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair
{
    int a;
    int b;
    __init(int x, int y) { a = x; b = y; }
}

typealias MyPair = Pair;

void main()
{
    MyPair p = MyPair(3, 4);
    // CHECK: p=34
    printf("p=%d%d\n", p.a, p.b);
}
