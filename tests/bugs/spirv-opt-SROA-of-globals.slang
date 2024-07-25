//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name
// Note: SROA will only run for 100 elements if spirv-opt does not manually set the spirv-opt `scalar-replacement` option
//CHECK-NOT: OpCompositeConstruct %Data
struct Data
{
    uint data0;
    uint data1;
    uint data2;
};

static Data globalVar;
ByteAddressBuffer bab;
RWStructuredBuffer<uint> outputBuffer;

struct Payload_t
{
    uint dataOut;
};

[shader("anyhit")]
void main1(inout Payload_t payload)
{
    globalVar = bab.Load<Data>(0);
    payload.dataOut = globalVar.data0;
}

[shader("anyhit")]
void main2(inout Payload_t payload)
{
    globalVar = bab.Load<Data>(0);
    payload.dataOut = globalVar.data0;
}