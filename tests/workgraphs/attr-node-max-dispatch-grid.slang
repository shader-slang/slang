//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that [NodeMaxDispatchGrid(x, y, z)] is emitted in HLSL output.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(16, 2, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK: [NodeMaxDispatchGrid(16, 2, 1)]
