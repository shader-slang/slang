//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %resourceHandles

[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind.General> resourceHandles[];

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
    __target_switch
    {
    case spirv:
        if (T.kind == DescriptorKind.Sampler) {
            return samplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
        }
        else {
            return resourceHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
        }
    default:
        return defaultGetDescriptorFromHandle(handle);
    }
}

// test.slang
struct PushConstant
{
    RWByteAddressBuffer.Handle test_buffer;
};
[[vk::push_constant]] PushConstant constant;

[shader("fragment")]
void main_test_fs(float2 fragment_coord: SV_Position, out float4 color: SV_Target)
{
    color = float4(constant.test_buffer.Load<uint3>(0), 1.0);
}
