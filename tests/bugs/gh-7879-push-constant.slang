//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK: OpCapability StoragePushConstant16

struct PushConstants {
    half4 color;
    int16_t index;
};

[[vk::push_constant]]
PushConstants pushConstants;

[shader("vertex")]
float4 vertexMain() : SV_POSITION
{
    return float4(pushConstants.color);
}