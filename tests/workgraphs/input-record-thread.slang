//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that ThreadNodeInputRecord<T> compiles as a thread-launch node shader parameter.

struct RecordData
{
    int value;
};

[shader("node")]
[NodeLaunch("thread")]
void main(ThreadNodeInputRecord<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get().value;
}

//CHK_DXIL: dx.op.createNodeInputRecordHandle
//CHK_DXIL: dx.op.getNodeRecordPtr
//CHK_HLSL: [NodeLaunch("thread")]
//CHK_HLSL: ThreadNodeInputRecord
//CHK_HLSL: input_0.Get().value_0
