//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#key-structkey
//META: doc_section_digest=2117d4dbe3c230380c22066ed6bf928ab2ce23e921d10d25ad32ec7e6e134733
//META: purpose=Field-access opcodes (get_field_addr) use the StructKey as the selector, confirming that StructKey is the identity of a field rather than a string name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc, field-access opcodes "use the key as their selector,
// which makes structural reorganization a key-rewriting task rather
// than a string-rewriting task". An `obj.x = ...` should emit
// `get_field_addr(%obj, %x)` — the second operand is the key.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int v;
uniform RWStructuredBuffer<int> buf;

struct Box
{
    int payload;
};

[numthreads(1, 1, 1)]
void main()
{
    Box b;
    b.payload = v;
    buf[0] = b.payload;
}

// CHECK: let  %payload	: _	= key
// CHECK: get_field_addr(%{{[A-Za-z0-9_]+}}, %payload)
