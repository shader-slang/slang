//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#dyn-interface-restrictions
//META: doc_section_digest=733bfc84d5a926cb0990d8b19fc9bdb571304a51dd223d8d0eb5820ebe12888a
//META: purpose=A type conforming to a `dyn interface` may not acquire the conformance through an `extension` (E33078) nor be generic (E33082), and its fields may be neither unsized (E33079), opaque (E33080), nor non-copyable (E33081).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the five conformer-side restrictions, which together are what force
// the dynamic representation to be a copyable, fixed-size box: an unsized, an
// opaque and a non-copyable field each break that box in a different way, and
// the extension and generic forms break the ability to fix the representation
// at the point of conformance. Each violation is a separate type so none masks
// another, and `-std 2026` opens the gate. CHECK pins one code per restriction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -std 2026

dyn interface IFoo
{
    int get();
}

struct ByExt
{
    int x;
}
extension ByExt : IFoo
//CHECK: E33078
{
    int get() { return x; }
}

struct GenConf<T> : IFoo
//CHECK: E33082
{
    int get() { return 0; }
}

struct Unsized : IFoo
{
    int tail[];
//CHECK: E33079
    int get() { return 0; }
}

struct Opaque : IFoo
{
    Texture2D t;
//CHECK: E33080
    int get() { return 0; }
}

[__NonCopyableType]
struct NC
{
    int v;
}

struct HasNC : IFoo
{
    NC n;
//CHECK: E33081
    int get() { return 0; }
}

void main()
{
}
