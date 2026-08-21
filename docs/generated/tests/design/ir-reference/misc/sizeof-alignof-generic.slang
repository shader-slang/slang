//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=b6a68a06f518f5bd44e3459811096c15e9f474df3116276f35e51d90ca68eef0
//META: purpose=`sizeof(T)` and `alignof(T)` on a generic type parameter emit `sizeOf` / `alignOf` insts carrying the type operand plus a data-layout operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two layout queries survive as insts when the natural
// layout cannot be computed yet, and that both spell out the optional
// second operand. Wrapping the queries in a generic keeps `T` unresolved at
// lowering time; the CHECKs pin `sizeOf(%T..., ScalarLayout)` and
// `alignOf(%T..., ScalarLayout)`, so an inst emitted with the type operand
// alone would not match.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

uint querySizeAlign<T>()
{
    return sizeof(T) + alignof(T);
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = querySizeAlign<float>();
    outputBuffer[1] = querySizeAlign<uint>();
}

// IR-LABEL: func %querySizeAlign
// IR-DAG: sizeOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
// IR-DAG: alignOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
