//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=c19c97466348045802dd2ac3f50daa2f7b25e6a69bb90b23bd259cb7cc63fb72
//META: purpose=A struct name is a DeclRefType referencing the StructDecl; a variable declared with that type stores the struct's fields and member access reads them.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a user-declared struct name is usable as a value type
// (the DeclRefType role) and that field/member resolution follows the
// underlying StructDecl. Fields are written then summed; CHECK asserts
// the sum, which is only reachable if each field access resolved
// through the decl-ref to its StructDecl member.

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
