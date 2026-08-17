//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Stress: Array composes with a user-struct element; the element-type operand names the struct IR identity.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Array composes with a user-struct element; the element-type operand
// names the struct IR identity. Combining `Array` with a user struct
// surfaces `Array(%Point, 4 : Int)` — the struct's IR identity sits in
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
