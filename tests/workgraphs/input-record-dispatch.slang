//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that DispatchNodeInputRecord<T> compiles as a node shader parameter.

struct RecordData
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get().value;
}

//CHK: void main(
