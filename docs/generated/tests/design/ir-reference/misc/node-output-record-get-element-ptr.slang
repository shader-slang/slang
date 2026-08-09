//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#work-graph-records-and-barrier-flags
//META: doc_section_digest=bfd67dc85af2750b4e7280782b8bb1df45e7ccceccc714257ca2c0031e18cf36
//META: purpose=Subscripting a node output-record collection lowers to `nodeOutputRecordGetElementPtr(base, index)` and is emitted as `base.Get(index)` in HLSL.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the record subscript is an address-producing operation, not a
// value read: only an l-value can be the target of the field assignments the
// shader performs. The IR directive requires the inst's result type to be a
// pointer to the record struct, and the HLSL directive requires the emitted
// form to be the `.Get(...)` accessor, which is HLSL's own l-value spelling
// for the same thing.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

struct OutData
{
    int value;
    int index;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(4, 1, 1)]
void main(uint threadIndex : SV_GroupIndex, [MaxRecords(4)] NodeOutput<OutData> threadOut)
{
    ThreadNodeOutputRecords<OutData> recs = threadOut.GetThreadNodeOutputRecords(1);
    recs[threadIndex].value = 42;
    recs[threadIndex].index = (int)threadIndex;
    recs.OutputComplete();
}

// IR: : Ptr(%OutData){{[ 	]*}}= nodeOutputRecordGetElementPtr(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})

//HLSL: recs{{[A-Za-z0-9_]*}}.Get(threadIndex{{[A-Za-z0-9_]*}}).value{{[A-Za-z0-9_]*}} = int(42);
