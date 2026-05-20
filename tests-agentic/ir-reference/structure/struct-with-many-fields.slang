//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#struct-internals
//META: doc_section_digest=512fdef1e28b21588ddc2979311636aaac69bfc4ae19f152ea896102f2378207
//META: purpose=A struct with multiple heterogeneous-typed fields emits one field child per declared field, each pairing its own key with its declared type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `struct Mixed { int a; float b; uint c; }` should emit three
// distinct `field(%key, T)` children under the `struct %Mixed` parent
// — one row per declared field, with each row pairing a unique key
// with the declared element type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct Mixed
{
    int a;
    float b;
    uint c;
};

[numthreads(1, 1, 1)]
void main()
{
    Mixed m;
    m.a = 1;
    m.b = 2.0;
    m.c = 3u;
    buf[0] = float(m.a) + m.b + float(m.c);
}

// CHECK: struct %Mixed	: Type
// CHECK: field(%a, Int)
// CHECK: field(%b, Float)
// CHECK: field(%c, UInt)
