//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#angle-bracket-annotations
//META: doc_section_digest=7ce4d4ac217c6c67d9cc75b8b918d498cbe8d315518ddcbd63412016bbc998ce
//META: purpose=The declarator-level annotation skip disambiguates first, so even under `-enable-effect-annotations` a `<let` prefix and a `< X :` prefix are still generic parameter lists.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two carve-outs that keep the opt-in flag from swallowing real
// generics. Both declarations here put a `<` immediately after a C-style
// function declarator — the exact position the annotation skip watches — and
// both are still read as generic parameter lists because of their opening
// tokens. The CHECK asserts the value parameter was specialized (the emitted
// multiply is by the literal 3, not by a runtime `N`), which only happens if
// `<let N : int>` bound a generic value rather than being discarded as an
// annotation; the interface-bounded call proves the same for the `< X :` form.

//TEST:SIMPLE(filecheck=CHECK):-enable-effect-annotations -target hlsl -entry main -stage compute

interface IThing { int id(); }
struct Impl : IThing { int id() { return 9; } }

float scaleN<let N : int>(float x) { return x * float(N); }
int idOf<T : IThing>(T t) { return t.id(); }

RWStructuredBuffer<float> outputBuffer;

//CHECK: float scaleN{{.*}}(float
//CHECK: 3.0

[numthreads(1, 1, 1)]
void main()
{
    Impl i;
    outputBuffer[0] = scaleN<3>(2.0) + float(idOf<Impl>(i));
}
