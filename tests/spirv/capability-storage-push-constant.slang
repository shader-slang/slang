//TEST:SIMPLE(filecheck=CHECK8): -target spirv -profile spirv_1_3 -DCONST_UINT8
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DCONST_UINT16
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DCONST_HALF
//TEST:SIMPLE(filecheck=CHECKBOTH): -target spirv -profile spirv_1_3 -DCONST_UINT8 -DCONST_HALF
//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK8: OpCapability StoragePushConstant8
//CHECK16: OpCapability StoragePushConstant16
//CHECKBOTH-DAG: OpCapability StoragePushConstant8
//CHECKBOTH-DAG: OpCapability StoragePushConstant16
//CHECK-NOT: OpCapability StoragePushConstant16

struct PushConstants {
#if defined(CONST_HALF)
    half4 color;
#else
    float4 color;
#endif
#if defined(CONST_UINT8)
    int8_t index;
#elif defined(CONST_UINT16)
    int16_t index;
#else
    int32_t index;
#endif
};

[[vk::push_constant]]
PushConstants pushConstants;

[shader("vertex")]
float4 vertexMain() : SV_POSITION
{
    return float4(pushConstants.color);
}
