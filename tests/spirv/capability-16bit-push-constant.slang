//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DCONST_HALF
//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DCONST_UINT16
//TEST:SIMPLE(filecheck=CHECKB): -target spirv -profile spirv_1_3

//CHECKA: OpCapability StoragePushConstant16
//CHECKB-NOT: OpCapability StoragePushConstant16

struct PushConstants {
#ifdef CONST_HALF
    half4 color;
#else
    float4 color;
#endif
#ifdef CONST_UINT16
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
