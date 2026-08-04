//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=A generic declaration after a decl keyword is unambiguous; the parser collects the parameter list into a GenericDecl and continues with the inner declaration.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `struct Box<T : IArithmetic>` is the unambiguous generic-decl case:
// after `struct` the `<` can only begin a generic parameter list. The
// inner struct is parsed and instantiated as `Box<int>` to confirm.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T : IArithmetic> { T v; };

void main()
{
    Box<int> b;
    b.v = 17;
    // CHECK: v=17
    printf("v=%d\n", b.v);
}
