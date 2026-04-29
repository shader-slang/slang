//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main

// Test that [NodeMaxDispatchGrid(x, y, z)] compiles to dxil and spirv.
// A broadcasting node with NodeMaxDispatchGrid must declare an input record with SV_DispatchGrid.

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(16, 2, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<GridData> input, RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_DXIL: i32 16, i32 2, i32 1
//CHK_SPIRV: OpFunction
