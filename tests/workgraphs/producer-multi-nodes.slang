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
    spirv_asm
    {
        OpExecutionMode $computeMain ShaderIndexAMDX $(0);
        OpExecutionMode $computeMain StaticNumWorkgroupsAMDX $(1) $(1) $(1);
    };

    //TODO: "myNodeID" is supposed to come from [NodeID("name")] attribute on "node" variable.
    // Until it is implemented properly, we will use an int-type generic argument.
    //
    #define myNodeID_1 0
    #define myNodeID_2 1

    ThreadNodeOutputRecords<RecordData, myNodeID_1> node1 = { 1 };
    ThreadNodeOutputRecords<RecordData, myNodeID_2> node2 = { 1 };

    node1.Get().myData = inputBuffer[0];
    node2.Get().myData = inputBuffer[0];

    node1.OutputComplete();
    node2.OutputComplete();
}

//CHK: OpCapability ShaderEnqueueAMDX
//CHK: OpExtension "SPV_AMDX_shader_enqueue"

//CHK: ; Annotations

//CHK-DAG: [[RecordType1:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX %RecordData
//CHK-DAG: [[RecordType2:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX %RecordData
//CHK-DAG: [[PtrType1:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[RecordType1]]
//CHK-DAG: [[PtrType2:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[RecordType2]]
//CHK-DAG: [[NodeID_1:%[a-zA-Z_0-9]+]] = OpConstantStringAMDX "NodeID_0"
//CHK-DAG: [[NodeID_2:%[a-zA-Z_0-9]+]] = OpConstantStringAMDX "NodeID_1"
//CHK-DAG: OpDecorateId [[RecordType1]] PayloadNodeNameAMDX [[NodeID_1]]
//CHK-DAG: OpDecorateId [[RecordType2]] PayloadNodeNameAMDX [[NodeID_2]]

//CHK-NOT: = OpTypeNodePayloadArrayAMDX
//CHK-NOT: = OpConstantStringAMDX
//CHK-NOT: OpDecorateID {{.*}} PayloadNodeNameAMDX

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK-DAG: [[Alloc1:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[PtrType1]] %
//CHK-DAG: [[Alloc2:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[PtrType2]] %
//CHK-NOT: = OpAllocateNodePayloadsAMDX

// ThreadNodeOutputRecords::Get()
//CHK-DAG: [[Load1:%[a-zA-Z_0-9]+]] = OpLoad [[PtrType1]] %var
//CHK-DAG: [[Load2:%[a-zA-Z_0-9]+]] = OpLoad [[PtrType2]] %var
//CHK-DAG: = OpAccessChain [[PtrType1]] [[Load1]] %
//CHK-DAG: = OpAccessChain [[PtrType2]] [[Load2]] %

// ThreadNodeOutputRecords::OutputComplete()
//CHK-DAG: OpEnqueueNodePayloadsAMDX [[Alloc1]]
//CHK-DAG: OpEnqueueNodePayloadsAMDX [[Alloc2]]
//CHK-NOT: OpEnqueueNodePayloadsAMDX

