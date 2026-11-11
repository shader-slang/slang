//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#ptr-and-the-access-qualifier--address-space-operands
//META: doc_section_digest=4eb81c51191334ef3e30616d7d16be9c32be2a38ef15ecc8cf34a2886b8386ae
//META: purpose=Boundary: a get_field_addr through a nested struct field yields a value of type Ptr(<InnerStruct>).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a get_field_addr through a nested struct field yields a value of
// type Ptr(<InnerStruct>). When a local nested-struct field is
// accessed by address, the inner `get_field_addr` produces a
// `Ptr(%Foo)` whose pointee is the inner struct type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
