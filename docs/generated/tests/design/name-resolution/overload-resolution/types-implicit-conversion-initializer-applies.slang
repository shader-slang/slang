//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=211bc03cd9a092841d1719f62552fc549d768b7b9dc4ea9b6c978e0acc8daa2b
//META: purpose=canCoerce accepts a single user-declared implicit-conversion initializer, so an A argument reaches a B parameter and the candidate becomes applicable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive half of the type step's coercion check for a
// user-declared conversion, and gives the nested-conversion negative test its
// baseline: one hop through B's __implicit_conversion initializer is allowed.
// CHECK asserts the value the initializer computed.

//TEST:INTERPRET(filecheck=CHECK):

struct A { int v; }
struct B { int v; __implicit_conversion(0) __init(A a) { v = a.v + 1; } }

int take(B b) { return b.v; }

void main()
{
    A a;
    a.v = 1;
    int r = take(a);
    // CHECK: r=2
    printf("r=%d\n", r);
}
