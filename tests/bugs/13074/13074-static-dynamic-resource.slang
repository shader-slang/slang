// Khronos legalization requires each cast from `__DynamicResource` to resolve to one module-scope
// dynamic-resource parameter or one indexed element. Replacing this mutable `static` variable with
// a function-local value could merge several such sources, so a later cast may no longer identify
// which one supplied its value. The front end therefore rejects the declaration on every target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

__DynamicResource inputResource;
static __DynamicResource cachedResource = inputResource;
//CHECK:                 ^^^^^^^^^^^^^^ opaque type is not supported for mutable static storage
//CHECK:                 ^^^^^^^^^^^^^^ mutable file- or namespace-scope `static` variable 'cachedResource' has the opaque type '__DynamicResource<__DynamicResourceKind.General>', which cannot be represented by mutable storage

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> texture = cachedResource.as<Texture2D<float4>>();
    output[0] = texture.Load(int3(0, 0, 0));
}
