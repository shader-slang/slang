//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=821d70752ab2fb0ab927aca034dc6d1ab1bf2642184d0e87594d63c182097322
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
