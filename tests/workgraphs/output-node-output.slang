//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test NodeOutput<T>, ThreadNodeOutputRecords<T>, IsValid(), and OutputComplete().

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> buf, [MaxRecords(4)] NodeOutput<OutputRecord> output)
{
    bool valid = output.IsValid();
    buf[0] = valid ? 1 : 0;
    ThreadNodeOutputRecords<OutputRecord> outRec = output.GetThreadNodeOutputRecords(1);
    outRec.OutputComplete();
}

//CHK_DXIL: dx.op.nodeOutputIsValid
//CHK_DXIL: dx.op.allocateNodeOutputRecords
//CHK_DXIL: dx.op.outputComplete
//CHK_HLSL: [MaxRecords(4)] NodeOutput
//CHK_HLSL: output_0.IsValid()
//CHK_HLSL: ThreadNodeOutputRecords
