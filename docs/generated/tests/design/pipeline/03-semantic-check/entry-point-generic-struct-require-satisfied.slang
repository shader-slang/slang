//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=c00647cfad21101d2bd2d831fcc3ac751988a6256cea5bd14c17a5aeb428a89e
//META: purpose=The same generic-struct requirement is accepted when the compile target does provide the required capability.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive side of the same check, so the negative test cannot
// pass for an unrelated reason. The identical program is compiled for the
// target the struct requires; CHECK asserts the emitted C++ contains the
// entry-point function, which only happens if validation let it through.

//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry main -stage vertex

[require(cpp)]
struct Foo<T>
{
    T value;
}

[shader("vertex")]
void main(Foo<int> a)
{
}
// CHECK: main
