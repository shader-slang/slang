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

//CHK-DAG: [[PayloadRecordType:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX %RecordData
//CHK-DAG: [[NodePtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadRecordType]]
//CHK-DAG: [[FuncPtrType:%[a-zA-Z_0-9]+]] = OpTypePointer Function %RecordData

//CHK-NOT: = OpTypeNodePayloadArrayAMDX
//CHK-NOT: = OpConstantStringAMDX
//CHK-NOT: OpDecorateID {{.*}} PayloadNodeNameAMDX

//CHK: ; Function

// ThreadNodeOutputRecords::__init()
//CHK: [[Alloc:%[a-zA-Z_0-9]+]] = OpAllocateNodePayloadsAMDX [[NodePtrType]] %
//CHK-NOT: = OpAllocateNodePayloadsAMDX

// ThreadNodeOutputRecords::Get()
//CHK: [[Load:%[a-zA-Z_0-9]+]] = OpLoad [[PayloadRecordType]]
//CHK: OpStore [[Var:%[a-zA-Z_0-9]+]] [[Load]]
//CHK: = OpAccessChain [[FuncPtrType]] [[Var]] %

// ThreadNodeOutputRecords::OutputComplete()
//CHK: OpEnqueueNodePayloadsAMDX [[Alloc]]
//CHK-NOT: OpEnqueueNodePayloadsAMDX

