//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#declref-family
//META: doc_section_digest=20c2eb71a7785d9324495ca71a49afdc3bcf69ae059047eeb0dd7dadf9b245cc
//META: purpose=Two GenericAppDeclRefs of the same generic decl with identical argument lists produce type-compatible references; a value of one is assignable to the other.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states `GenericAppDeclRef` is "a generic decl-ref with
// its arguments applied". When two appearances of `Box<int>`
// supply identical generic arguments, they refer to the same
// type and assignment is accepted in both directions.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T> { T v; }

void main()
{
    Box<int> a;
    a.v = 7;
    Box<int> b = a;       // same generic args -> assignable
    Box<int> c;
    c = b;                // same generic args -> assignable
    // CHECK: v=7,7,7
    printf("v=%d,%d,%d\n", a.v, b.v, c.v);
}
