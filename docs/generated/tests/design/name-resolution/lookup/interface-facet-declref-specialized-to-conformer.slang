//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=8db1d44af39131b43cc8258b2bec4dad2a4381ae619907da2e2b6a9de2021028
//META: purpose=FacetImpl::init specializes an interface facet's parent decl-ref with _maybeSpecializeSuperTypeDeclRef, so member lookup gives a requirement returning This the concrete conforming type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `FacetImpl::init` calls `_maybeSpecializeSuperTypeDeclRef` to build the interface facet's parent
// decl-ref from `ICloneable`'s `ThisTypeDecl` and the conformance witness.
// `_lookUpDirectAndTransparentMembers` uses that parent when it constructs the `clone` result.
// Because `clone` returns `This`, calling it on `S` must therefore yield `S`, not the abstract self
// type. CHECK confirms that the returned value has `S.v == 42`.

//TEST:INTERPRET(filecheck=CHECK):

interface ICloneable { This clone(); }

struct S : ICloneable
{
    int v;
    This clone() { S s; s.v = v + 1; return s; }
}

void main()
{
    S s;
    s.v = 41;
    S t = s.clone();
    // CHECK: r=42
    printf("r=%d\n", t.v);
}
