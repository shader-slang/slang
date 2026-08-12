//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#struct-internals
//META: doc_section_digest=b9a3eeb4f30459f951dba340f8e24ccd1d1bf7aa9598b948a2a6457d2453d958
//META: purpose=A struct with heterogeneously-typed fields emits one field child per declared field, each pairing its own key with its declared type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercises the type axis of the `field` row: the `fieldType` operand is the
// member's own declared type, not a normalised one. Three members of three
// different scalar types are declared and the ordered CHECKs require
// `field(%a, Int)`, `field(%b, Float)` and `field(%c, UInt)` in declaration
// order under the one `struct %Mixed` parent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;
uniform int seed;

struct Mixed
{
    int a;
    float b;
    uint c;
}

[numthreads(1, 1, 1)]
void main()
{
    Mixed m;
    m.a = seed;
    m.b = float(seed) * 2.0;
    m.c = uint(seed);
    buf[0] = float(m.a) + m.b + float(m.c);
}

// CHECK: struct %Mixed	: Type
// CHECK: field(%a, Int)
// CHECK: field(%b, Float)
// CHECK: field(%c, UInt)
