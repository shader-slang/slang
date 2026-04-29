//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

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

//CHK: dx.op.createNodeOutputHandle
//CHK: dx.op.nodeOutputIsValid
//CHK: dx.op.incrementOutputCount
