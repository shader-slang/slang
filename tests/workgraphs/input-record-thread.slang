//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that ThreadNodeInputRecord<T> compiles as a thread-launch node shader parameter.

struct RecordData
{
    int value;
};

[shader("node")]
[NodeLaunch("thread")]
[numthreads(1, 1, 1)]
void main(ThreadNodeInputRecord<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get().value;
}

//CHK: [NodeLaunch("thread")]
//CHK: void main(
