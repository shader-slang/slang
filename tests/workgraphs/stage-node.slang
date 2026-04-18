//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

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

//CHK: [shader("node")]
//CHK: [numthreads(1, 1, 1)]
//CHK: void main(
