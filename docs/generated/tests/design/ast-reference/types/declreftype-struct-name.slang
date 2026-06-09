//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=354aa5e4ad824810f192137554fb86892215b9326ffe9fad3435b95088dbb52d
//META: purpose=A struct name is a DeclRefType referencing the StructDecl; a variable declared with that type stores the struct's fields and member access reads them.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `DeclRefType` notable-node section says "almost every
// user-declared type the front end sees is a DeclRefType ... carries
// a decl-ref to a type declaration". The user-spelled struct name
// `Vec3i` resolves through a DeclRefType to its StructDecl; field
// access then dispatches via the decl-ref.

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
