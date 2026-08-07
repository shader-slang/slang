//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

struct Record
{
    uint value;
};

void writeUInt(out uint value)
{
    value = 7;
}

void updateUInt(inout uint value)
{
    value += 1;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(1)] NodeOutput<Record> output)
{
    int value = 0;
    writeUInt(value);
    updateUInt(value);

    GroupNodeOutputRecords<Record> records = output.GetGroupNodeOutputRecords(1);
    records.Get().value = (uint)value;
    records.OutputComplete();

    Barrier(
        (BarrierMemoryTypeFlags)(0x1 | 0x4),
        (BarrierSemanticFlags)(0x1 | 0x2));
}

//CHK: writeUInt_
//CHK: updateUInt_
//CHK: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY), (GROUP_SYNC | GROUP_SCOPE));
