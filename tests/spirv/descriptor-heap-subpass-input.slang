// Regression test for shader-slang/slang#12680: SubpassInput indexed from a descriptor heap under
// spvDescriptorHeapEXT must lower to a subpass-data image descriptor fetched from the resource heap.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -profile spirv_1_6 -capability spvDescriptorHeapEXT -emit-spirv-directly

struct Push { uint colorIndex; uint depthIndex; };
[[vk::push_constant]] ConstantBuffer<Push> push;

[shader("fragment")]
float4 main() : SV_Target0
{
    // CHECK-DAG: OpCapability InputAttachment
    // OpTypeImage operands are: sampled-type, Dim, Depth, Arrayed, MS, Sampled, Format. Here
    // Depth=2 (no depth indication), Arrayed=0, MS=0 (single) / MS=1 (multisampled), Sampled=2
    // (read without a sampler), Format=Unknown.
    // CHECK-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float SubpassData 2 0 0 2 Unknown
    // CHECK-DAG: %[[IMG_MS:[A-Za-z0-9_]+]] = OpTypeImage %float SubpassData 2 0 1 2 Unknown
    // CHECK-DAG: %[[ARR:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[IMG]]
    // CHECK-DAG: %[[ARR_MS:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[IMG_MS]]
    // CHECK-DAG: OpUntypedAccessChainKHR %{{[A-Za-z0-9_]+}} %[[ARR]] %slang_resourceHeap
    // CHECK-DAG: OpUntypedAccessChainKHR %{{[A-Za-z0-9_]+}} %[[ARR_MS]] %slang_resourceHeap
    // The multisampled read carries a Sample image operand; the single-sampled read does not.
    // CHECK-DAG: OpImageRead %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Sample %{{[A-Za-z0-9_]+}}
    SubpassInput<float4> color = ResourceDescriptorHeap[push.colorIndex];
    SubpassInputMS<float4> depth = ResourceDescriptorHeap[push.depthIndex];
    return color.SubpassLoad() + depth.SubpassLoad(0);
}
