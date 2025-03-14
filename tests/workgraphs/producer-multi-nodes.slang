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

//CHK: ; Types, variables and constants

//CHK-DAG: [[MemberType:%[a-zA-Z_0-9]+]] = OpTypeInt 32 1
//CHK-DAG: [[StructType:%[a-zA-Z_0-9]+]] = OpTypeStruct [[MemberType]]
//CHK-DAG: [[PayloadType1:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK-DAG: [[PayloadType2:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK-DAG: [[NodePtrType1:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType1]]
//CHK-DAG: [[NodePtrType2:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType2]]
//CHK-DAG: [[TempPtrType1:%[a-zA-Z_0-9]+]] = OpTypePointer Function [[PayloadType1]]
//CHK-DAG: [[TempPtrType2:%[a-zA-Z_0-9]+]] = OpTypePointer Function [[PayloadType2]]
//CHK-DAG: [[FuncPtrType:%[a-zA-Z_0-9]+]] = OpTypePointer Function [[StructType]]

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK-DAG: [[TempPtrVar1:%[a-zA-Z_0-9]+]] = OpVariable [[TempPtrType1]] Function
//CHK-DAG: [[TempPtrVar2:%[a-zA-Z_0-9]+]] = OpVariable [[TempPtrType2]] Function
//CHK-DAG: [[Alloc1:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType1]] %
//CHK-DAG: [[Alloc2:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType2]] %
//CHK-DAG: [[Loaded1:%[a-zA-Z_0-9]+]] = OpLoad [[PayloadType1]] [[Alloc1]]
//CHK-DAG: [[Loaded2:%[a-zA-Z_0-9]+]] = OpLoad [[PayloadType2]] [[Alloc2]]
//CHK-DAG: OpStore [[TempPtrVar1]] [[Loaded1]]
//CHK-DAG: OpStore [[TempPtrVar2]] [[Loaded2]]

// ThreadNodeOutputRecords::Get()
//CHK-COUNT-2: = OpAccessChain [[FuncPtrType]] %

// ThreadNodeOutputRecords::OutputComplete()
//CHK-COUNT-2: OpEnqueueNodePayloadsAMDX %

