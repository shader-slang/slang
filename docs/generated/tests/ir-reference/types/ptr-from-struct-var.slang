//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#ptr-and-the-access-qualifier-address-space-operands
//META: doc_section_digest=24583cce8243a26e6a6077b224e031406f0c718c4c1df3a220b3be0cd7b9c590
//META: purpose=A local var of struct type produces a value whose IR type is Ptr<T>.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "`Ptr` and the access-qualifier / address-space
// operands" note says `Ptr(valueType, ...)` is the IR pointer-type
// encoding. A local struct variable accessed by field address
// keeps its `var` form and the result is typed `Ptr(<StructName>)`
// in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Foo f;
    f.x = a;
    f.y = a + 1;
    buf[0] = f.x + f.y;
}

// IR-LABEL: func %main
// IR: let {{.*}}%f{{.*}}: Ptr(%Foo){{.*}}= var
