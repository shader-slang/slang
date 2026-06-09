//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=354aa5e4ad824810f192137554fb86892215b9326ffe9fad3435b95088dbb52d
//META: purpose=A DeclRefType carries the generic substitutions that apply to its declaration; two specializations of a generic struct hold values typed as the substituted parameter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `DeclRefType` note states: "A `DeclRefType` carries a
// decl-ref to a type declaration ... together with whatever generic
// substitutions apply." A `Pair<int>` and a `Pair<float>` are
// distinct `DeclRefType` instances of the same `StructDecl`; the
// fields carry the substituted scalar type.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair<T> { T first; T second; }

void main()
{
    Pair<int> pi;
    pi.first = 100;
    pi.second = 200;
    Pair<float> pf;
    pf.first = 1.5f;
    pf.second = 2.5f;
    // CHECK: pi=100,200
    printf("pi=%d,%d\n", pi.first, pi.second);
    // CHECK: pf=1,2
    printf("pf=%d,%d\n", int(pf.first), int(pf.second));
}
