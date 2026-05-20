//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=Stress: Array composes with a user-struct element; the element-type operand names the struct IR identity.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Arrays" row says the element-type operand is any
// IR type. Combining `Array` with a user struct surfaces
// `Array(%Point, 4 : Int)` — the struct's IR identity sits in
// the first operand slot.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Point { float x; float y; }
struct Bag { Point points[4]; }

uniform Bag b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = b.points[0].x;
}

// IR-LABEL: struct %Bag
// IR: field(%{{.*}}, Array(%Point, 4 : Int))
