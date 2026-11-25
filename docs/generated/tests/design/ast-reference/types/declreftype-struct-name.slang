//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=5cf72875eab2bff5179bb24a0df3a152b019d5d2fb8764c0ea77dc305e749a24
//META: purpose=A struct name is a type defined by reference to its declaration; a variable declared with that name stores the struct's fields and member access resolves through the declaration.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a user-declared struct name is usable directly as a
// value type and that member lookup follows the referenced declaration.
// Three fields are written then summed; CHECK asserts the sum, which is
// only reachable if each `.x`/`.y`/`.z` resolved through the decl-ref to
// its own member.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec3i { int x; int y; int z; }

void main()
{
    Vec3i v;
    v.x = 11;
    v.y = 22;
    v.z = 33;
    // CHECK: sum=66
    printf("sum=%d\n", v.x + v.y + v.z);
}
