//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#key--structkey
//META: doc_section_digest=d2ea86e1e20890cc6353bda3406625017f20105e88f02a8d8e91e8431b3fecb3
//META: purpose=Field-access opcodes use the StructKey as their selector, confirming the key rather than a string name is the identity of a field.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that field access is keyed, which is what makes struct
// reorganization a key-rewriting task. Writing `b.payload` emits
// `get_field_addr(<base>, %payload)`; CHECK first requires `%payload` to be a
// `key` inst and then requires that same value to appear in the selector
// position of the access, so the access cannot be resolving by name.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int v;
uniform RWStructuredBuffer<int> buf;

struct Box
{
    int payload;
}

[numthreads(1, 1, 1)]
void main()
{
    Box b;
    b.payload = v;
    buf[0] = b.payload;
}

// CHECK: let  %payload	: _	= key
// CHECK: get_field_addr(%{{[A-Za-z0-9_]+}}, %payload)
