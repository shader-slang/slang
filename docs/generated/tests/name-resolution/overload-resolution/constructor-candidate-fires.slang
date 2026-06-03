//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=25b7fa875eaa3f334cbe42c411b97e9677fd0d8ee67a01fca43e03581635f93c
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
