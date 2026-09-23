// Khronos legalization requires each cast from `__DynamicResource` to resolve to one module-scope
// dynamic-resource parameter or one indexed element. Replacing this mutable `static` variable with
// a function-local value could merge several such sources, so a later cast may no longer identify
// which one supplied its value. The front end therefore rejects the declaration on every target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

__DynamicResource inputResource;
static __DynamicResource cachedResource = inputResource;
//CHECK:                 ^^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                 ^^^^^^^^^^^^^^ global variable cannot have opaque type.

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> texture = cachedResource.as<Texture2D<float4>>();
    output[0] = texture.Load(int3(0, 0, 0));
}
