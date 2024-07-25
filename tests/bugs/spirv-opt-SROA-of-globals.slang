//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name

// Note: SROA will only run for 100 elements if spirv-opt does not manually set the spirv-opt `scalar-replacement` option
//CHECK-NOT: OpCompositeConstruct
struct Data 
{
uint data0;
uint data1;
uint data2;
uint data3;
uint data4;
uint data5;
uint data6;
uint data7;
uint data8;
uint data9;
uint data10;
uint data11;
uint data12;
uint data13;
uint data14;
uint data15;
uint data16;
uint data17;
uint data18;
uint data19;
uint data20;
uint data21;
uint data22;
uint data23;
uint data24;
uint data25;
uint data26;
uint data27;
uint data28;
uint data29;
uint data30;
uint data31;
uint data32;
uint data33;
uint data34;
uint data35;
uint data36;
uint data37;
uint data38;
uint data39;
uint data40;
uint data41;
uint data42;
uint data43;
uint data44;
uint data45;
uint data46;
uint data47;
uint data48;
uint data49;
uint data50;
uint data51;
uint data52;
uint data53;
uint data54;
uint data55;
uint data56;
uint data57;
uint data58;
uint data59;
uint data60;
uint data61;
uint data62;
uint data63;
uint data64;
uint data65;
uint data66;
uint data67;
uint data68;
uint data69;
uint data70;
uint data71;
uint data72;
uint data73;
uint data74;
uint data75;
uint data76;
uint data77;
uint data78;
uint data79;
uint data80;
uint data81;
uint data82;
uint data83;
uint data84;
uint data85;
uint data86;
uint data87;
uint data88;
uint data89;
uint data90;
uint data91;
uint data92;
uint data93;
uint data94;
uint data95;
uint data96;
uint data97;
uint data98;
uint data99;
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