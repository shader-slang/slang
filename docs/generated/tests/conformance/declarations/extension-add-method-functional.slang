//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#extensions
//META: doc_section_digest=ec41e5f3bc9673746ac30b0bb1f19e54c7c9d47f0377ab73cc170b70580c168d
//META: purpose=An extension declaration adds behavior to an existing type; instance and static methods added via extension are accessible through the extended type (instance method returns x*x+y*y; static method returns dimensionality).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an extension adds both instance and static methods to an existing struct.
// `getLengthSq()` is called on a `MyVec` instance and `getDimensionality()` on the type itself;
// CHECK lines assert 25 and 2, proving both extension members dispatch correctly.

//TEST:INTERPRET(filecheck=CHECK):

struct MyVec { float x, y; }

extension MyVec
{
    float getLengthSq() { return x*x + y*y; }
    static int getDimensionality() { return 2; }
}

void main()
{
    MyVec v;
    v.x = 3.0;
    v.y = 4.0;
    // CHECK: 25
    printf("%g\n", v.getLengthSq());
    // CHECK: 2
    printf("%d\n", MyVec.getDimensionality());
}
