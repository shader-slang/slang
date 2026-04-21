//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that GroupNodeOutputRecords compiles and emits correctly.

struct MyRecord
{
    uint value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    DispatchNodeInputRecord<MyRecord> input,
    [MaxRecords(1)] NodeOutput<MyRecord> output)
{
    GroupNodeOutputRecords<MyRecord> rec = output.GetGroupNodeOutputRecords(1);
    rec.Get().value = input.Get().value;
    rec.OutputComplete();
}

//CHK: GroupNodeOutputRecords
//CHK: GetGroupNodeOutputRecords
