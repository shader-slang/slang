//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#deduplication
//META: doc_section_digest=802de1508256e26daf66c46a07bf5d1f9a2f9a83d5b8d75510a0d79a00768b98
//META: purpose=Lookup returns the interface requirements alongside the concrete member that satisfies them, and the caller's narrowing keeps only the concrete member so the call is not ambiguous.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lookup deliberately keeps competing items and lets the
// caller narrow them. S conforms to two interfaces that each require
// `f()`, and one concrete `f()` satisfies both, so the receiver call
// sees three candidates: two requirements and the implementation.
// CHECK asserts 7, meaning narrowing selected the concrete member
// instead of reporting an ambiguity.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int f(); }
interface IB { int f(); }

struct S : IA, IB { int f() { return 7; } }

void main()
{
    S s;
    // CHECK: r=7
    printf("r=%d\n", s.f());
}
