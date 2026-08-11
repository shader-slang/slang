//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=The link-time alias spelling `struct FooAlias : IFoo = Foo;` makes the alias name usable as a conforming type whose interface methods dispatch to the aliased type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the link-time alias form an aggregate declaration accepts on its
// right-hand side. `FooAlias` is declared with `: IFoo = Foo`; a value of the
// alias type is created and its interface method is called. The CHECK asserts
// the value `Foo::get` returns, proving the alias resolved to `Foo` rather
// than to an empty placeholder type.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo { int get(); }

struct Foo : IFoo { int get() { return 41; } }

struct FooAlias : IFoo = Foo;

void main()
{
    FooAlias f;
    //CHECK: 41
    printf("%d\n", f.get());
}
