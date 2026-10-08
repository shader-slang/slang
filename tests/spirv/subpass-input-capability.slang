//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -stage fragment -entry main
//TEST:SIMPLE(filecheck=NOCAP):-target spirv -emit-spirv-directly -stage fragment -entry main

// A shader whose only unformatted image accesses are subpass loads must not
// require StorageImageReadWithoutFormat: Dim=SubpassData is excluded from that
// requirement, and InputAttachment alone covers the access.
// See: https://github.com/shader-slang/slang/issues/12474

// Own prefix so this spans the whole module; the forbidden capability would be
// emitted right after InputAttachment, so sharing the CHECK prefix would scope
// it to before the first positive match and pass vacuously.
// NOCAP-NOT: OpCapability StorageImageReadWithoutFormat

// CHECK-DAG: OpCapability InputAttachment
// CHECK-DAG: OpTypeImage %float SubpassData 2 0 0 2 Unknown
// CHECK-DAG: OpTypeImage %float SubpassData 2 0 1 2 Unknown
// CHECK-COUNT-2: OpImageRead

[[vk::input_attachment_index(0)]] SubpassInput<float4> subpass;
[[vk::input_attachment_index(1)]] SubpassInputMS<float4> subpassMS;

[shader("fragment")]
float4 main() : SV_Target
{
    return subpass.SubpassLoad() + subpassMS.SubpassLoad(0);
}
