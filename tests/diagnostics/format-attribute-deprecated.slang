//DIAGNOSTIC_TEST:SIMPLE(diag=no-diag): -std 2025 -target spirv -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=warn): -std 2026 -target spirv -stage compute -entry computeMain

[format("rgba8")]
/*warn:
 ^^^^^^ use of deprecated [format(...)] attribute
 ^^^^^^ [format(...)] has been deprecated; use explicit texture storage types such as `RWTexture2D<RGBA8Unorm>` instead. See https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/02-conventional-features.html#texture-types
*/
RWTexture2D<float4> colorTexture;

[[vk::image_format("r32ui")]]
/*warn:
      ^^^^^^^^^^^^ use of deprecated [format(...)] attribute
      ^^^^^^^^^^^^ [format(...)] has been deprecated; use explicit texture storage types such as `RWTexture2D<RGBA8Unorm>` instead. See https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/02-conventional-features.html#texture-types
*/
RWTexture2D<uint> counterTexture;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    colorTexture[dispatchThreadID.xy] = float4(0.0f);
    counterTexture[dispatchThreadID.xy] = 1u;
}
