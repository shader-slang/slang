//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/misc.md#type-queries-and-predicates
//META: doc_section_digest=871b3f331a04a078de5515922d4952968bc581652a6602e75bcb911f396e2550
//META: purpose=The `is` operator on an existential parameter lowers to the `IsType` IR opcode with `(value, valueWitness, typeOperand, targetWitness)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A dynamic interface parameter and an `is` test against a
// concrete impl is the doc-named surface for `IsType`. The lowering
// extracts the existential value/witness first, then emits
// `IsType(%val, %witness, %TargetType, %targetWitness)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { float area(); }
struct Circle : IShape { float r; float area() { return r * r; } }
struct Square : IShape { float s; float area() { return s * s; } }

uniform RWStructuredBuffer<int> buf;

int classify(IShape shape)
{
    if (shape is Circle) return 1;
    if (shape is Square) return 2;
    return 0;
}

[numthreads(1, 1, 1)]
void main()
{
    Circle c = { 1.0 };
    buf[0] = classify(c);
}

// IR-LABEL: func %classify
// IR: IsType({{.*}}, {{.*}}, %Circle, {{.*}})
// IR: IsType({{.*}}, {{.*}}, %Square, {{.*}})
