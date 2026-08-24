//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=4d65a5f477addb92a25c5896b6cb496d8436da7a5ccfb3b367db94c8861a7ee9
//META: purpose=Assigning to a swizzle whose base is a `property` with `get`/`set` but no `ref` accessor produces `swizzleSet` (a get, a functional update, then a set) instead of `swizzledStore`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies which of the two swizzle-assignment opcodes lowering picks.
// The choice hangs on whether the swizzle's base can be turned into a
// pointer: `s.v.xy = ...` writes through the field's address and takes
// the in-place `swizzledStore` path (covered by
// swizzle-store-lvalue.slang), while `s.P.xy = ...` goes through a
// property that offers only `get` and `set`, so there is no address to
// store into. CHECK asserts the read-modify-write triple — a call to
// the getter, a `swizzleSet` naming lanes 0 and 1, then a call to the
// setter — and that no `swizzledStore` appears, which is what proves
// the non-addressable base selected the functional-update form.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 gv;
uniform float fa;
uniform RWStructuredBuffer<float4> v4buf;

struct S
{
    float4 v;
    property P : float4 { get { return v; } set { v = newValue; } }
}

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s.v = gv;
    s.P.xy = float2(fa, fa);
    v4buf[0] = s.v;
}

// IR-LABEL: func %main
// IR: call %Sx5FPx5Fget(
// IR: swizzleSet(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, 0 : Int, 1 : Int)
// IR: call %Sx5FPx5Fset(
// IR-NOT: swizzledStore
