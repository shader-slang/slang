//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that GroupNodeInputRecords<T> compiles as a coalescing-launch node shader parameter,
// including parameter-level [MaxRecords(n)] emission.

struct RecordData
{
    int value;
};

[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(1, 1, 1)]
void main([MaxRecords(8)] GroupNodeInputRecords<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get(0).value;
}

//CHK_DXIL: dx.op.createNodeInputRecordHandle
//CHK_DXIL: dx.op.getNodeRecordPtr
//CHK_DXIL: !{i32 1, i32 65, i32 2, !{{[0-9]+}}, i32 3, i32 8}
//CHK_HLSL: [MaxRecords(8)] GroupNodeInputRecords
//CHK_HLSL: input_0.Get(0U).value_0
