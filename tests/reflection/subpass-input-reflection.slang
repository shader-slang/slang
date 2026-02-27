// subpass-input-reflection.slang
//
// Regression test for GitHub issue #10045:
// SubpassInput caused a crash during reflection JSON emission on Linux
// because SLANG_PARAMETER_CATEGORY_SUBPASS fell into the default switch
// case in emitReflectionVarBindingInfoJSON, and SubpassInputType was not
// handled in the reflection API (GetKind, GetResourceShape, GetResourceResultType).

//TEST:REFLECTION(filecheck=CHECK):-target spirv -no-codegen

[[vk::input_attachment_index(0)]] SubpassInput<float4> subpassinput_0;

[shader("fragment")]
float4 main() : SV_Target
{
    float4 color = subpassinput_0.SubpassLoad();
    return float4(color.rgb, 1);
}

// CHECK: "name": "subpassinput_0"
// CHECK: "kind": "inputAttachmentIndex"
// CHECK: "index": 0
// CHECK: "baseShape": "subpassInput"
// CHECK: "resultType"
