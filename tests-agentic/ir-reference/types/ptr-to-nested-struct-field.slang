//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#ptr-and-the-access-qualifier-address-space-operands
//META: doc_section_digest=24583cce8243a26e6a6077b224e031406f0c718c4c1df3a220b3be0cd7b9c590
//META: purpose=Boundary: a get_field_addr through a nested struct field yields a value of type Ptr(<InnerStruct>).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Ptr` note says `Ptr(valueType, ...)` is the IR
// pointer-type encoding. When a local nested-struct field is
// accessed by address, the inner `get_field_addr` produces a
// `Ptr(%Foo)` whose pointee is the inner struct type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; }
struct Bar { Foo f; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Bar b;
    b.f.x = a;
    b.y = a + 1;
    buf[0] = b.f.x + b.y;
}

// IR: let {{.*}}: Ptr(%Foo){{.*}}= get_field_addr
