//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#istype
//META: doc_section_digest=96c88cd229bf1685831b41b4f853c066f396c97bc43dfff2bd5daafb4bb81fe0
//META: purpose=The `is` operator on an existential value lowers to a four-operand `IsType(value, valueWitness, typeOperand, targetWitness)` inst whose result type is `bool`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the type test carries witnesses alongside the target type,
// which is what later lets the dynamic-dispatch lowering turn it into a
// witness comparison instead of a type comparison. A function taking an
// interface-typed parameter tests it against two different concrete structs;
// each CHECK requires four operands with the concrete type in the third
// position and a `Bool` result, so a three-operand or type-only encoding would
// not match.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { float area(); }
struct Circle : IShape { float r; float area() { return r * r; } }
struct Square : IShape { float s; float area() { return s * s; } }

RWStructuredBuffer<int> outputBuffer;

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
    outputBuffer[0] = classify(c);
}

// IR-LABEL: func %classify
// IR-DAG: : Bool{{[ 	]*}}= IsType(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, %Circle, %{{[A-Za-z0-9_]+}})
// IR-DAG: : Bool{{[ 	]*}}= IsType(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, %Square, %{{[A-Za-z0-9_]+}})
