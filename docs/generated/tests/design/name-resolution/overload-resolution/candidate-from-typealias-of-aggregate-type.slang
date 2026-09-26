//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=fd6b141e684f15a00cb062b04ca96b050f6b3c8c740707d6c23327611bd617b5
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
