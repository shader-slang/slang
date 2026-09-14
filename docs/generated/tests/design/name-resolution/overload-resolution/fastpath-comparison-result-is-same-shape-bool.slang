//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=A comparison handled by the fast path produces the same-shape bool type, so int3 == int3 has type vector<bool,3>.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the fast path gives a comparison the *same-shape* bool result
// type rather than a scalar bool. An overload probe on `bool` versus `bool3`
// observes the type of `a == b`; CHECK asserts the bool3 overload fired.

//TEST:INTERPRET(filecheck=CHECK):

int probe(bool x)  { return 1; }
int probe(bool3 x) { return 3; }

void main()
{
    int3 a = int3(1, 2, 3);
    int3 b = int3(1, 0, 3);
    int tag = probe(a == b);
    // CHECK: tag=3
    printf("tag=%d\n", tag);
}
