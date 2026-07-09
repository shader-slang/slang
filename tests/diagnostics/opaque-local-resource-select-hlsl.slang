// Issue #10526: the opaque-local-variable restriction is specific to Khronos
// targets (SPIR-V/GLSL). Selecting a resource through control flow is perfectly
// valid for HLSL/D3D, so the same source that is rejected for SPIR-V must
// compile cleanly here. This pins the `isKhronosTarget` gate against a polarity
// regression that would otherwise spuriously reject valid HLSL.

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
