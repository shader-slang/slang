//TEST:SIMPLE(filecheck=CHK): -target spirv-asm -stage compute -entry computeMain -skip-spirv-validation

RWStructuredBuffer<int> inputBuffer;

struct RecordData
{
    int myData;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{

    //TODO: "myNodeID" is supposed to come from [NodeID("name")] attribute on "node" variable.
    // Until it is implemented properly, we will use an int-type generic argument.
    //
    #define myNodeID 0
    ThreadNodeOutputRecords<RecordData, myNodeID> node = { 1 };

    node.Get().myData = inputBuffer[0];
    node.OutputComplete();
}

//CHK: OpCapability ShaderEnqueueAMDX
//CHK: OpExtension "SPV_AMDX_shader_enqueue"

//CHK: ; Annotations

//CHK-DAG: [[RecordType:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX %RecordData
//CHK-DAG: [[PtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[RecordType]]
//CHK-DAG: [[NodeID:%[a-zA-Z_0-9]+]] = OpConstantStringAMDX "NodeID_0"
//CHK-DAG: OpDecorateId [[RecordType]] PayloadNodeNameAMDX [[NodeID]]

//CHK-NOT: = OpTypeNodePayloadArrayAMDX
//CHK-NOT: = OpConstantStringAMDX
//CHK-NOT: OpDecorateID {{.*}} PayloadNodeNameAMDX

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK: [[Alloc:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[PtrType]] %
//CHK-NOT: = OpAllocateNodePayloadsAMDX

// ThreadNodeOutputRecords::Get()
//CHK: [[Load:%[a-zA-Z_0-9]+]] = OpLoad [[PtrType]] %var
//CHK: = OpAccessChain [[PtrType]] [[Load]] %

// ThreadNodeOutputRecords::OutputComplete()
//CHK: OpEnqueueNodePayloadsAMDX [[Alloc]]
//CHK-NOT: OpEnqueueNodePayloadsAMDX

