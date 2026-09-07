//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=bd2e5c9c313f8eee04427c042995baa864cc9e2068805e81fdd68950019c18ec
//META: purpose=In `JustTrying` mode the overload-time visibility check silently drops an inaccessible candidate instead of diagnosing, so a visible overload can win a call that the private one matched better.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the silent-drop half of the overload-time check. From outside the
// struct, `s.compute(3)` matches the `private int compute(int)` overload
// exactly, but that candidate is dropped without a diagnostic; the surviving
// `internal float compute(float)` wins through an implicit conversion. The
// CHECK asserts the float result 6.0 rather than the int overload's 30,
// which is the observable difference between dropping the candidate and
// diagnosing it.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    private int compute(int x) { return x * 10; }
    internal float compute(float x) { return x * 2.0; }
}

void main()
{
    S s;
    // CHECK: r=6.000000
    printf("r=%f\n", s.compute(3));
}
