//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS): -experimental-feature -entry nativeAlias -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=COMPUTE): -experimental-feature -ignore-capabilities -entry nativeAlias -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -experimental-feature -entry nativeGeneric<InputCarrier> -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=HELPER): -experimental-feature -entry Stage.helper -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=PLAIN): -experimental-feature -entry Plain.invoke -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Stage.invoke -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Stage -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativePhantom -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativeStagePayload -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativeOrdinaryResource -stage closesthit -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

typealias HitInput = rt::ClosestHitInput<HitContext>;

// Type aliases do not turn a logical input into a native parameter.
// The second alias run selects compute to check that this rule is not limited to native RT stages.
void nativeAlias(in HitInput input) {}
/*ALIAS:
                    ^^^^^^^^ invalid native entry point signature
                    ^^^^^^^^ cannot be used in a native entry point signature
*/
/*COMPUTE:
                    ^^^^^^^^ invalid native entry point signature
                    ^^^^^^^^ cannot be used in a native entry point signature
*/

interface IInputCarrier
{
    associatedtype Item;
}

struct InputCarrier : IInputCarrier
{
    typealias Item = HitInput;
}

// This entry selection already supplies InputCarrier. Check the substituted signature; checking
// only the original declaration would see T.Item instead of the concrete stage input.
void nativeGeneric<T : IInputCarrier>(in T.Item input) {}
/*GENERIC:
                                           ^^^^ invalid native entry point signature
                                           ^^^^ cannot be used in a native entry point signature
*/

// Helpers may still copy and forward the input supplied by a structural invocation.
void forwardInput(in HitInput input)
{
    let copy = input;
    copy.payload += 1;
}

struct Stage : rt::IClosestHitShader
{
    typealias Context = HitContext;

    // Both names select the same checked structural implementation and allow its logical input.
    static void invoke(in HitInput input)
    {
        forwardInput(input);
    }

    // Other methods remain native entries even when their containing type implements a stage.
    static void helper(in HitInput input) {}
/*HELPER:
                          ^^^^^^^^ invalid native entry point signature
                          ^^^^^^^^ cannot be used in a native entry point signature
*/
}

struct Plain
{
    // The method name alone does not supply an executable stage conformance.
    static void invoke(in HitInput input) {}
/*PLAIN:
                          ^^^^^^^^ invalid native entry point signature
                          ^^^^^^^^ cannot be used in a native entry point signature
*/
}

struct Phantom<T>
{
    uint value;
}

// A type argument or associated type is not itself a runtime field. InputCarrier's Item must not
// cause this ordinary payload to be rejected when no instance field stores that associated type.
void nativePhantom(inout Phantom<InputCarrier> payload) {}

// The resource's represented element fields are ordinary data. InputCarrier is only a phantom
// type argument here, so visiting every generic argument would incorrectly reject this binding.
void nativeOrdinaryResource(uniform ConstantBuffer<Phantom<InputCarrier>> input) {}

// A stage implementation is an ordinary user struct; its static invoke is not a payload field.
void nativeStagePayload(inout Stage payload) {}
