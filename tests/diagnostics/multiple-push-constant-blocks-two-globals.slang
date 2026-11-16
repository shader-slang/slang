// Two global push-constant blocks that one entry point reads are rejected
// (shader-slang/slang#13713), including when one block is a
// `ConstantBuffer<T>` that the entry point reaches only through a helper
// function, and for a non-compute stage.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry vmain -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry vmain -stage vertex

struct PC
{
    float4 a;
}

struct PC2
{
    float4 b;
}

[[vk::push_constant]]
uniform PC push;
//CHECK:   ^^^^ push-constant block `push` is used by this entry point

[[vk::push_constant]]
ConstantBuffer<PC2> push2;
//CHECK:            ^^^^^ push-constant block `push2` is used by this entry point

float4 readSecond()
{
    return push2.b;
}

[shader("vertex")]
float4 vmain(float3 p: POSITION) : SV_Position
/*CHECK:
       ^^^^^ error E39033
       ^^^^^ entry point `vmain` uses more than one push-constant block
       ^^^^^ combine the data into one
*/
{
    return float4(p, 1) + push.a + readSecond();
}
