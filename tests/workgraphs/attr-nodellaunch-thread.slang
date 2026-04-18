//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that [NodeLaunch("thread")] is emitted in HLSL output.

[shader("node")]
[NodeLaunch("thread")]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK: [NodeLaunch("thread")]
