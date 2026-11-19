// A reverse-differentiated function that takes a resource-typed parameter gets a
// synthesized `apply` call, and autodiff marks that call with an
// [IgnoreSideEffects] decoration because it is a pure re-execution of the primal
// code. When the resource parameter needs type legalization, the legalizer
// rebuilds the call, and the rebuilt call must keep the decoration:
// `mightHaveSideEffects` reads it when deciding whether dead-code elimination may
// remove the call, so dropping it silently changes optimization behavior. Pin the
// decoration on the rebuilt call in the post-legalization IR. Autodiff also
// decorates the extracted primal function and its non-context parameters with
// the same decoration, and the filecheck pattern anchors to the first occurrence
// in the dump; the rebuilt apply call appears before the apply function's
// definition here, so the decoration check lands on the call, and the
// CHECK-NEXT binds it to the apply call line (a decoration elsewhere cannot
// satisfy that pair).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -dump-ir-after legalizeResourceTypes

uniform Texture2D tex;
uniform SamplerState samp;

struct TW { Texture2D t; };

[Differentiable]
float f(float x, TW wrapper)
{
    float4 c = no_diff(wrapper.t).Sample(samp, float2(x, 0.5));
    return c.x * x;
}

RWStructuredBuffer<float> outBuf;

[shader("fragment")]
float4 main(float4 c : SV_Target)
{
    var p = diffPair(2.0f, 0.0f);
    bwd_diff(f)(p, TW(tex), 1.0f);
    outBuf[0] = p.d;
    return c;
}

//CHECK: ### AFTER legalizeResourceTypes:
//CHECK: [ignoreSideEffectsDecoration]
//CHECK-NEXT: call %sx5Fapplyx5Ff
