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

//CHK: ; Types, variables and constants

//CHK-DAG: [[MemberType:%[a-zA-Z_0-9]+]] = OpTypeInt 32 1
//CHK-DAG: [[StructType:%[a-zA-Z_0-9]+]] = OpTypeStruct [[MemberType]]
//CHK-DAG: [[PayloadType:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK-DAG: [[NodePtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType]]
//CHK-DAG: [[TempPtrType:%[a-zA-Z_0-9]+]] = OpTypePointer Function [[PayloadType]]
//CHK-DAG: [[FuncPtrType:%[a-zA-Z_0-9]+]] = OpTypePointer Function [[StructType]]

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK: [[TempPtrVar:%[a-zA-Z_0-9]+]] = OpVariable [[TempPtrType]] Function
//CHK: [[Alloc:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType]] %
//CHK: [[Loaded:%[a-zA-Z_0-9]+]] = OpLoad [[PayloadType]] [[Alloc]]
//CHK: OpStore [[TempPtrVar]] [[Loaded]]

// ThreadNodeOutputRecords::Get()
//CHK: = OpAccessChain [[FuncPtrType]] %

// ThreadNodeOutputRecords::OutputComplete()
//CHK: OpEnqueueNodePayloadsAMDX %

