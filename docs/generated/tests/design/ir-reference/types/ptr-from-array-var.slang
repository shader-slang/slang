//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#ptr-and-the-access-qualifier-address-space-operands
//META: doc_section_digest=24583cce8243a26e6a6077b224e031406f0c718c4c1df3a220b3be0cd7b9c590
//META: purpose=Boundary: a local var of fixed-array type produces a value of IR type Ptr(Array(<T>, <N>)).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Ptr` note says `Ptr(valueType, ...)` is the pointer
// type opcode. A local fixed-size array variable is allocated
// via `var` and its result is typed `Ptr(Array(Int, 3 : Int))` —
// composition between `Ptr` and `Array`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int xs[3];
    xs[0] = a;
    xs[1] = a + 1;
    xs[2] = a + 2;
    buf[0] = xs[0] + xs[1] + xs[2];
}

// IR-LABEL: func %main
// IR: let {{.*}}%xs{{.*}}: Ptr(Array(Int, 3 : Int)){{.*}}= var
