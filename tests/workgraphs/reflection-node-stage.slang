//TEST:REFLECTION(filecheck=CHECK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -no-codegen
import experimental.workgraph;

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<GridData> input)
{
}

//CHECK: "name": "main",
//CHECK-NEXT: "stage": "node",
