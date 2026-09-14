//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=eb0fd6263e6c05fcda56c26f5dbe0c80c738c9165b8bfd51abc3dc4235097f39
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
