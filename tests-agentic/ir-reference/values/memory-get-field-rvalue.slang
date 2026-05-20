//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#fieldaddress-vs-fieldextract
//META: doc_section_digest=8630a844543700b647b31dc8de2111393a6e2d61ce3abe98c96f921ac5c4e59f
//META: purpose=A `MemberExpr` on a struct *value* (rvalue position) lowers to `get_field`, while the lvalue path uses `get_field_addr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `FieldAddress` vs `FieldExtract` notable-opcode note
// distinguishes the lvalue (`get_field_addr`) and rvalue
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
