//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=7dc220c4c4dd5c2b21296a00da8c0b8bb7caf5c69af13f6b032f0ae0e4047ecd
//META: purpose=An alignof of a type is a compile-time integer value, distinct from the sizeof form, and is strictly positive and no larger than the type's size.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `alignof` is its own compile-time integer alongside
// `sizeof`. Both initialize `static const int`s, which requires each to
// be evaluated during checking; CHECK asserts the alignment is positive
// and does not exceed the size, the only relation that holds on every
// target (the absolute numbers are target-dependent).

//TEST:INTERPRET(filecheck=CHECK):

struct Pair { int a; int b; }

void main()
{
    static const int AL = int(alignof(Pair));
    static const int SZ = int(sizeof(Pair));
    // CHECK: align_ok=1
    printf("align_ok=%d\n", (AL > 0 && AL <= SZ) ? 1 : 0);
}
