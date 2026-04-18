//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that the SM 6.8 Barrier(uint, uint) intrinsic and flag constants compile.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> buf)
{
    buf[0] = 1;
    Barrier(UAV_MEMORY | NODE_INPUT_MEMORY, GROUP_SYNC | DEVICE_SCOPE);
}

//CHK: Barrier(
