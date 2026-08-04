//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=72472a48bac619938006bb23a0e1ce51075ed1e262b7eb32a455b8764ff2fc94
//META: purpose=Two GenericDecls can compose: a generic struct field of a generic struct specializes when the outer specializes.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that nested generic decls compose: the outer GenericDecl
// specializes its inner field type, which itself is a generic.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T> { T value; }

struct Pair<T> { Box<T> first; Box<T> second; }

void main()
{
    Pair<int> p;
    p.first.value  = 3;
    p.second.value = 4;
    //CHECK: 7
    printf("%d\n", p.first.value + p.second.value);
}
