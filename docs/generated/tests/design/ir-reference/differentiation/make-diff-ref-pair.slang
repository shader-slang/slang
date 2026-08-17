//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-construction
//META: doc_section_digest=94baa99b60cb82fe486d311fcf1a6ed19cf37b8c6ff65ecb372a31812c6e6208
//META: purpose=The DifferentialPtrPair<T> constructor lowers to MakeDiffRefPair, the pointer-typed sibling of MakeDiffPair.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second row of the construction table: a pointer-typed primal
// bundled with a pointer-typed differential lowers to `MakeDiffRefPair`, a
// distinct opcode from the value-typed `MakeDiffPair`. `MyPtrType` conforms to
// `IDifferentiablePtrType` (the conformance `DifferentialPtrPair` requires) and
// each half is a different window into the same buffer so the two operands stay
// distinct values. CHECK pins the named local's `DiffRefPair(%MyPtrType, ...)`
// result type together with the `MakeDiffRefPair` producer, which is what
// distinguishes this opcode from its value-typed sibling.

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
    buf[0] = pp.p.load(0);
}

// CHECK: func %main
// CHECK: let %pp{{.*}}DiffRefPair(%MyPtrType,{{.*}}= MakeDiffRefPair(
