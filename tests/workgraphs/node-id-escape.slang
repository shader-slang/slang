//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
[NodeID("Entry\"Node")]
void main([NodeID("Target\"Node", 0)] NodeOutput<OutputRecord> output)
{
}

//CHK: [NodeID("Entry\"Node", 0)]
//CHK: [NodeID("Target\"Node", 0)] NodeOutput
