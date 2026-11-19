//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry fragmentMain -stage fragment

// `[vk::offset]` places uniform data, so it is an error on a varying
// entry-point parameter.

[shader("fragment")]
float4 fragmentMain(
    [vk::offset(16)] float4 color : COLOR,
//CHECK: ^^^^^^ '[vk::offset]' on a varying entry-point parameter
//CHECK: ^^^^^^ '[vk::offset]' places uniform data, so it is not allowed on 'color', which is a varying (stage input/output) parameter
    [vk::offset(4)] uniform float scale) : SV_Target
{
    return color * scale;
}
