//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#loweredvalinfo-and-the-lowering-environment
//META: doc_section_digest=fccfd69fda9694cd247e946541d540d87908b0504cf18d5e9a98277654d151b6
//META: purpose=Reading a property backed by a `get` accessor forces the symbolic `BoundStorage` form down to a single value by emitting a call to the accessor function at the use site.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accessor-call half of the compound-lowered-value story. A
// property has no storage, so `c.doubled` cannot lower to a field address; it
// stays bound to its accessor set until the read site decides to fetch a value.
// The CHECKs assert the getter is lowered as its own function and that the read
// in `main` is a call to it, while the plain field `c.raw` next to it still
// lowers to a direct `get_field_addr` — the contrast shows the accessor call is
// property-specific, not a general struct-member effect.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct Counter
{
    int raw;

    property int doubled
    {
        get { return raw * 2; }
        set { raw = newValue / 2; }
    }
}

[numthreads(1, 1, 1)]
void main()
{
    Counter c;
    c.raw = a;
    buf[0] = c.doubled;
}

// CHECK: [nameHint("Counter.doubled.get")]
// CHECK-NEXT: func [[GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: func %main
// CHECK: get_field_addr(%c,
// CHECK: call [[GET]](
