//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that function-level [MaxRecords(n)] is emitted for coalescing nodes.

struct MyRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("coalescing")]
[MaxRecords(4)]
[numthreads(4, 1, 1)]
void main(GroupNodeInputRecords<MyRecord> input)
{
    uint count = input.Count();
}

//CHK: [MaxRecords(4)]
//CHK: void main
