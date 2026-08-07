//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-projection
//META: doc_section_digest=5f50033280aec505eacac53ad283962baab159487ef0677b633a1b40a4acdceb
//META: purpose=The .p and .d getters of DifferentialPtrPair<T> lower to GetPrimalRef and GetDifferentialPtr, the pointer-flavoured projections.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two pointer-flavoured rows of the projection table, which the
// doc separates from the value-flavoured pair because they project a
// `DifferentialPtrPair` rather than a `DifferentialPair`. The same pair value
// is projected both ways in one entry point and both results are summed into
// the buffer so neither projection is dead. CHECK-DAG is used because the two
// projections are independent and their relative dump order is not the property
// under test; each pattern pins the projected operand `%pp` so a match cannot
// drift onto some other pair.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct MyPtrType : IDifferentiablePtrType
{
    typealias Differential = MyPtrType;

    RWStructuredBuffer<float> buffer;
    uint offset;

    float load(uint idx) { return buffer[offset + idx]; }
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPtrPair<MyPtrType> pp = DifferentialPtrPair<MyPtrType>({buf, 0}, {buf, 2});
    buf[0] = pp.p.load(0) + pp.d.load(1);
}

// CHECK: func %main
// CHECK-DAG: = GetPrimalRef(%pp)
// CHECK-DAG: = GetDifferentialPtr(%pp)
