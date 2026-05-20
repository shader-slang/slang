//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup on a pointer-like ConstantBuffer<T> auto-dereferences to the pointee and finds the inner field through a Deref breadcrumb.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_lookUpMembersInSuperTypeImpl` calls
// `getPointedToTypeIfCanImplicitDeref(superType)` and prepends a
// Deref breadcrumb when the type is pointer-like. ConstantBuffer<T>
// is pointer-like; `cb.a` must resolve to `T`'s field `a` via the
// auto-deref path.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

struct Params { float a; }

ConstantBuffer<Params> cb;

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    // CHECK: a_{{[0-9]+}}
    outBuf[0] = cb.a;
}
