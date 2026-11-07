// W38044 flags varying parameters of an entry point that owns module-scope varyings. A builtin
// parameter type such as a geometry shader's `TriangleStream<T>` has no module-scope equivalent,
// so it is not flagged. A compute shader's system-value parameter is flagged like any other
// varying parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=GEOMETRY):-target spirv -entry main -stage geometry -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=COMPUTE):-lang glsl tests/diagnostics/entry-point-param-with-global-varyings-stages.slang -target spirv -entry main -stage compute -no-codegen -DUSE_COMPUTE

#ifndef USE_COMPUTE
in triangle float4 gCoarse[3];

struct RasterVertex
{
    float4 position : SV_Position;
};

[maxvertexcount(3)]
void main(inout TriangleStream<RasterVertex> stream)
{
    for (int i = 0; i < 3; i++)
    {
        RasterVertex v;
        v.position = gCoarse[i];
        stream.Append(v);
    }
}
#else
layout(location = 0) in vec2 texCoord;
/*COMPUTE:
                             ^^^^^^^^ module-scope varying 'texCoord' is declared here
*/

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*COMPUTE:
                ^^^^^^^^^^^^^^^^ entry point has varying parameters and also owns module-scope varyings
                ^^^^^^^^^^^^^^^^ 'dispatchThreadID' is a varying parameter of an entry point that also owns module-scope varyings; this will be an error in a future compiler release
*/
{
}
#endif
