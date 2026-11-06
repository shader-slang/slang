// A `ConstantBuffer<P>` passed to a `__constref P` parameter from an exported
// function, with no entry point, used to crash the compiler
// (https://github.com/shader-slang/slang/issues/13656).

//TEST:SIMPLE(filecheck=CUDA): -target cuda
//TEST:SIMPLE(filecheck=METAL): -target metal

// CUDA: exportedGetColor
// METAL: exportedGetColor

struct Params
{
    float4 color;
};

ConstantBuffer<Params> gParams;

float4 getColor(__constref Params p)
{
    return p.color;
}

export float4 exportedGetColor()
{
    return getColor(gParams);
}
