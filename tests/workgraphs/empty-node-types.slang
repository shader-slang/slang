//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that EmptyNodeInput, EmptyNodeOutput, and EmptyNodeOutputArray compile
// and emit correctly.

struct MyRecord
{
    uint value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
    EmptyNodeInput input,
    EmptyNodeOutput outputA,
    EmptyNodeOutputArray outputArr)
{
    uint count = input.Count();
    if (outputA.IsValid())
        outputA.OutputComplete();
    EmptyNodeOutput elem = outputArr[0];
    if (elem.IsValid())
        elem.OutputComplete();
}

//CHK: EmptyNodeInput
//CHK: EmptyNodeOutput
//CHK: EmptyNodeOutputArray
