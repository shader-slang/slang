// Regression test for https://github.com/shader-slang/slang/issues/13817 (generic witness case).
//
// `make_target` wraps the associated type of a dynamic `IMaterial` with two conformances in the
// generic `Target<T> : ITarget`. When the result is used as an `ITarget`, the `ITarget` witness
// table is `Target<T>`'s conformance specialized over the set of `IBSDF` witness tables, and its
// `value` entry needs a runtime tag that the caller has no way to receive. The compiler must
// report E33180 at the `value()` call instead of crashing, both when `use<T : ITarget>` receives
// the value as a generic argument and when the value is stored in an `ITarget` existential.

//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0" -conformance "B:IMaterial=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target hlsl -entry computeMain -stage compute -conformance "A:IMaterial=0" -conformance "B:IMaterial=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=EXISTENTIAL):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0" -conformance "B:IMaterial=1" -DUSE_EXISTENTIAL

// With a single conformance the material is monomorphic, so both spellings compile.
//TEST:SIMPLE(filecheck=SINGLE):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0"
//TEST:SIMPLE(filecheck=SINGLE):-target hlsl -entry computeMain -stage compute -conformance "A:IMaterial=0"
//TEST:SIMPLE(filecheck=SINGLE):-target spirv -entry computeMain -stage compute -conformance "A:IMaterial=0" -DUSE_EXISTENTIAL
// SINGLE-NOT: error[
// SINGLE: computeMain

interface IBSDF { float eval(); }
struct BSDFA : IBSDF { float a; float eval() { return a; } }
struct BSDFB : IBSDF { float b; float c; float eval() { return b * c; } }

interface IMaterial { associatedtype BSDF : IBSDF; BSDF get_bsdf(); }
struct A : IMaterial { typealias BSDF = BSDFA; float4 data; BSDFA get_bsdf() { return { data.x }; } }
struct B : IMaterial { typealias BSDF = BSDFB; float4 data; BSDFB get_bsdf() { return { data.x, data.y }; } }

interface ITarget { float value(); }
struct Target<T : IBSDF> : ITarget { T bsdf; float value() { return bsdf.eval(); } }
Target<T> make_target<T : IBSDF>(const T bsdf) { return { bsdf }; }

float use<T : ITarget>(const T target)
{
    return target.value();
//GENERIC:             ^ cannot specialize generic with existential type
//GENERIC:             ^ specializing 'Target.value' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
}

StructuredBuffer<float4> materials;
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const IMaterial material = createDynamicObject<IMaterial>(tid.x & 1, materials[tid.x]);
#ifdef USE_EXISTENTIAL
    ITarget target = make_target(material.get_bsdf());
    outputBuffer[tid.x] = target.value();
//EXISTENTIAL:                        ^ cannot specialize generic with existential type
//EXISTENTIAL:                        ^ specializing 'Target.value' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
#else
    outputBuffer[tid.x] = use(make_target(material.get_bsdf()));
#endif
}
