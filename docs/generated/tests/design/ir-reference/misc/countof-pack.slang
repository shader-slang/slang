//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=b6a68a06f518f5bd44e3459811096c15e9f474df3116276f35e51d90ca68eef0
//META: purpose=`countof(pack)` on a variadic value pack lowers to a `countOf` inst whose operand is the pack itself.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pack form of the count query, which is the one the doc singles
// out as taking the pack value as its operand. The count is asked inside a
// generic so the pack length is still symbolic at lowering, and the CHECK
// requires the inst's operand to be the generic's pack parameter rather than a
// folded literal.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int queryCount<let each D : int>()
{
    return countof(D);
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = queryCount<1, 2, 3>();
    outputBuffer[1] = queryCount<10, 20, 30, 40>();
}

// IR-LABEL: func %queryCount
// IR: countOf(%D{{[A-Za-z0-9_]*}})
