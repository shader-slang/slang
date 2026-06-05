//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that EmptyNodeInput, EmptyNodeOutput, and EmptyNodeOutputArray compile
// and emit correctly.

// EmptyNodeInput is valid for coalescing-launch nodes; EmptyNodeOutput and
// EmptyNodeOutputArray are valid for any launch type.
[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4, 1, 1)]
void main(
    EmptyNodeInput input,
    EmptyNodeOutput outputA,
    EmptyNodeOutputArray outputArr)
{
    uint count = input.Count();
    if (outputA.IsValid())
        outputA.ThreadIncrementOutputCount(1);
    EmptyNodeOutput elem = outputArr[0];
    if (elem.IsValid())
        elem.ThreadIncrementOutputCount(1);
}

//CHK_DXIL: dx.op.createNodeOutputHandle
//CHK_DXIL: dx.op.nodeOutputIsValid
//CHK_DXIL: dx.op.incrementOutputCount
//CHK_HLSL: EmptyNodeInput input_0
//CHK_HLSL: EmptyNodeOutput outputA_0
//CHK_HLSL: EmptyNodeOutputArray outputArr_0
//CHK_HLSL: input_0.Count()
//CHK_HLSL: outputA_0.ThreadIncrementOutputCount(1U);
