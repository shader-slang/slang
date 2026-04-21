//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that [NodeLaunch("thread")] is emitted and [numthreads] is not required
// for thread-launch nodes.

[shader("node")]
[NodeLaunch("thread")]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK: [NodeLaunch("thread")]
//CHK-NOT: numthreads
