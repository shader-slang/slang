//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

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
    if (count != 0)
        outputA.GroupIncrementOutputCount(count);
    if (outputA.IsValid())
        outputA.ThreadIncrementOutputCount(1);
    EmptyNodeOutput elem = outputArr[0];
    if (elem.IsValid())
        elem.ThreadIncrementOutputCount(1);
}

//CHK: dx.op.createNodeOutputHandle
//CHK: dx.op.getInputRecordCount
//CHK: dx.op.nodeOutputIsValid
//CHK: dx.op.incrementOutputCount
//CHK_HLSL: EmptyNodeInput input
//CHK_HLSL: EmptyNodeOutput outputA
//CHK_HLSL: EmptyNodeOutputArray outputArr
//CHK_HLSL: .Count()
