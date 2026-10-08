// Issue #10526: the HLSL emitter can preserve this resource selection in source form without losing
// information. It must therefore emit the source and leave acceptance to the downstream compiler,
// rather than applying a restriction from targets that cannot represent the surviving local.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment

Texture1D<float> tex0, tex1;

[noinline]
Texture1D<float> returnHelper(Texture1D<float> a, Texture1D<float> b, bool useSecond)
{
    return useSecond ? b : a;
}

[shader("fragment")]
float main(bool useSecond) : SV_Target
{
    Texture1D<float> returned = returnHelper(tex0, tex1, useSecond);
    return returned[0];
}

// CHECK-NOT: opaque type in local variable
// CHECK: main(
