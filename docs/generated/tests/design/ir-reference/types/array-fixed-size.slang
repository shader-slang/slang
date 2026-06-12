//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=A fixed-size array field appears as Array(elementType, elementCount) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Arrays" table row reads
// "`Array` | `ArrayType` | `elementType: IRType, elementCount, stride?`".
// A struct-field of type `float[8]` and a local `int[3]` together
// surface `Array(Float, 8 : Int)` and `Array(Int, 3 : Int)` in the
// dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Bag { float data[8]; }

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    Bag b;
    b.data[0] = a;
    b.data[1] = a + 1;
    b.data[7] = a + 7;

    int xs[3];
    xs[0] = int(a);
    xs[1] = int(a) + 1;
    xs[2] = int(a) + 2;

    buf[0] = b.data[1] + float(xs[2]);
}

// IR-LABEL: struct %Bag
// IR: field(%{{.*}}, Array(Float, 8 : Int))
// IR-LABEL: func %main
// IR: let {{.*}}%xs{{.*}}: Ptr(Array(Int, 3 : Int)){{.*}}= var
