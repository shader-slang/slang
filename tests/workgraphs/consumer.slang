//TEST:SIMPLE(filecheck=CHK): -target spirv-asm -stage compute -entry computeMain -skip-spirv-validation

RWStructuredBuffer<int> outputBuffer;

struct RecordData
{
    int myData;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadId : SV_GroupThreadID)
{
    spirv_asm
    {
        OpExecutionMode $computeMain ShaderIndexAMDX $(0);
        OpExecutionMode $computeMain StaticNumWorkgroupsAMDX $(1) $(1) $(1);
    };

    //TODO: "myNodeID" is supposed to come from [NodeID("name")] attribute of the entry point.
    // Until it is implemented properly, we will use an int-type generic argument.
    //
    #define myNodeID 0
    DispatchNodeInputRecord<RecordData, myNodeID> inputData;

    int myData = inputData.Get().myData;
    outputBuffer[dispatchThreadId.x] = myData;
}

//CHK: OpCapability ShaderEnqueueAMDX
//CHK: OpExtension "SPV_AMDX_shader_enqueue"

//CHK: ; Types, variables and constants
//CHK: [[MemberType:%[a-zA-Z_0-9]+]] = OpTypeInt 32 1
//CHK: [[StructType:%[a-zA-Z_0-9]+]] = OpTypeStruct [[MemberType]]
//CHK: [[PayloadType:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK: [[PtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType]]

//CHK: ; Function
//CHK: [[VarName:%[a-zA-Z_0-9]+]] = OpVariable [[PtrType]] NodePayloadAMDX
//CHK: = OpAccessChain [[PtrType]] [[VarName]]

