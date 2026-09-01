//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that [NodeMaxDispatchGrid(x, y, z)] compiles to dxil.
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
//CHK_HLSL: [NodeMaxDispatchGrid(16, 2, 1)]
//CHK_HLSL: DispatchNodeInputRecord
