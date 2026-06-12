//TEST:SIMPLE(filecheck=CHK_MAX): -target hlsl -stage node -entry maxGridMain -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_FIXED): -target hlsl -stage node -entry fixedGridMain -profile lib_6_8

// Test that the maximum valid node grid dimension is accepted.

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(65535, 65535, 65535)]
[numthreads(1, 1, 1)]
void maxGridMain(DispatchNodeInputRecord<GridData> input, RWBuffer<int> output)
{
    output[0] = input.Get().grid.x;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[numthreads(1, 1, 1)]
void fixedGridMain(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_MAX: [NodeMaxDispatchGrid(65535, 65535, 65535)]
//CHK_FIXED: [NodeDispatchGrid(65535, 1, 1)]
