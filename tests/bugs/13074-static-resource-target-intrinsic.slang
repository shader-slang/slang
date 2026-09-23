// A target-intrinsic definition replaces its function's body only when the definition applies to
// the current target. Resource-global legalization must reject the applicable HLSL definition
// below, while allowing the CUDA-only function whose body is emitted for HLSL.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture;

__target_intrinsic(hlsl, "float4(0.0, 0.0, 0.0, 0.0)")
float4 useHLSLIntrinsic()
/*CHECK:
       ^^^^^^^^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a target-intrinsic function
       ^^^^^^^^^^^^^^^^ function 'useHLSLIntrinsic' has a target-intrinsic implementation for this compilation target, so Slang emits that implementation instead of the function body and cannot preserve the body's access to a resource declared static at file or namespace scope; pass the resource as an explicit function argument and reference it from the target intrinsic instead
*/
{
    return cachedTexture.Load(int3(0, 0, 0));
}

__target_intrinsic(cuda, "make_float4(0.0f, 0.0f, 0.0f, 0.0f)")
float4 useCUDAIntrinsic()
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTexture = inputTexture;
    return useHLSLIntrinsic() + useCUDAIntrinsic();
}
