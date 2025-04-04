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

//CHK: ; Types, variables and constants

//CHK-DAG: [[MemberType:%[a-zA-Z_0-9]+]] = OpTypeInt 32 1
//CHK-DAG: [[StructType:%[a-zA-Z_0-9]+]] = OpTypeStruct [[MemberType]]
//CHK-DAG: [[PayloadType1:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK-DAG: [[PayloadType2:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK-DAG: [[NodePtrType1:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType1]]
//CHK-DAG: [[NodePtrType2:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType2]]
//CHK-DAG: [[StructPtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[StructType]]

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK-DAG: [[Alloc1:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType1]] %
//CHK-DAG: [[Alloc2:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType2]] %

// ThreadNodeOutputRecords::Get()
//CHK: = OpAccessChain [[StructPtrType]] [[Alloc1]]
//CHK: = OpAccessChain [[StructPtrType]] [[Alloc2]]

// ThreadNodeOutputRecords::OutputComplete()
//CHK: OpEnqueueNodePayloadsAMDX [[Alloc1]]
//CHK: OpEnqueueNodePayloadsAMDX [[Alloc2]]

