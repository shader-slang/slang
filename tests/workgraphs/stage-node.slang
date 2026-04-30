//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main -skip-spirv-validation

// Test that Stage::Node compiles as a lib_6_8 library shader.

struct RecordData { int value; };

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get().value;
}

//CHK_DXIL: dx.op.createNodeInputRecordHandle
//CHK_DXIL: dx.op.getNodeRecordPtr
//CHK_SPIRV: OpTypeNodePayloadArrayAMDX
