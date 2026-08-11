//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#cooperative-matrix-and-vector
//META: doc_section_digest=5d8bfb7cd06d16d7e5526799f2be56c9cee4a73edf35171033b8a239608e92f4
//META: purpose=`coopVecMatMulAdd` on a `CoopVec` with an optional bias surfaces the documented `CoopVecMatMulAdd` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a cooperative-vector matrix-multiply-add collapses into
// one opcode carrying the input vector, the matrix pointer, and the
// layout/interpretation parameters. The result is stored so it cannot
// be eliminated, and the CHECK looks for the opcode in the user body.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;
RWStructuredBuffer<half> sink;

[numthreads(1, 1, 1)]
void main()
{
    CoopVec<half, 4> vec = coopVecLoad<4, half>(input);
    let result = coopVecMatMulAdd<half, 4, 4>(
        vec,
        CoopVecComponentType.Float16,
        matrix,
        0,
        CoopVecComponentType.Float16,
        bias,
        0,
        CoopVecComponentType.Float16,
        CoopVecMatrixLayout.RowMajor,
        false,
        8);

    for (int i = 0; i < result.getCount(); ++i)
        sink[i] = result[i];
}

// IR-LABEL: func %main
// IR: CoopVecMatMulAdd
