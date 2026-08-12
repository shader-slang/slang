//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#fieldaddress-vs-fieldextract
//META: doc_section_digest=c51c6f434f2f289530bd44c1c012d7d1ae5c9ab1af33f21f78b660210957b529
//META: purpose=A `MemberExpr` on a struct *value* (rvalue position) lowers to `get_field`, while the lvalue path uses `get_field_addr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the distinction between `FieldAddress` and `FieldExtract`, which separates the lvalue (`get_field_addr`) and rvalue
// (`get_field`) paths. A `uniform` struct global is a value, so
// reading its fields surfaces `get_field` (the cross-cutting bundle
// covers `get_field_addr` via a local struct).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Mixed { int x; float y; bool z; }

uniform Mixed s;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.x;
    fbuf[0] = s.y;
    buf[1] = select(s.z, 1, 0);
}

// IR-LABEL: func %main
// IR: get_field(%s, %{{.*}})
// IR: get_field(%s, %{{.*}})
// IR: get_field(%s, %{{.*}})
