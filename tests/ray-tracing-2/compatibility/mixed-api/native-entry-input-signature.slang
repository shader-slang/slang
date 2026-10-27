//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS): -experimental-feature -entry nativeAlias -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=COMPUTE): -experimental-feature -ignore-capabilities -entry nativeAlias -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=OPTIONAL): -experimental-feature -entry nativeOptional -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=UNIFORM): -experimental-feature -entry nativeUniform -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -experimental-feature -entry nativeField -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE): -experimental-feature -ignore-capabilities -entry nativeResource -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=STRUCTURED_RESOURCE): -experimental-feature -ignore-capabilities -entry nativeStructuredResource -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=RESULT): -experimental-feature -entry nativeResult -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -experimental-feature -entry nativeGeneric<InputCarrier> -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=METHOD): -experimental-feature -entry Stage.invoke -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=INHERITED): -experimental-feature -DINHERITED -entry nativeInherited -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Stage -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativePhantom -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativeStagePayload -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry nativeOrdinaryResource -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -ignore-capabilities -entry nativeRecursive -stage closesthit -no-codegen

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

// Type aliases and optional wrappers do not turn a logical input into a native parameter.
// The second alias run selects compute to check that this rule is not limited to native RT stages.
void nativeAlias(in HitInput input) {}
/*ALIAS:
                    ^^^^^^^^ invalid native entry point signature
                    ^^^^^^^^ cannot be used in a native entry point signature
*/
/*COMPUTE:
                    ^^^^^^^^ invalid native entry point signature
                    ^^^^^^^^ cannot be used in a native entry point signature
                             ^^^^^ entry point parameter treated as uniform
                             ^^^^^ parameter 'input' is treated as 'uniform'
*/

void nativeOptional(in Optional<Optional<HitInput>> input) {}
/*OPTIONAL:
                       ^^^^^^^^ invalid native entry point signature
                       ^^^^^^^^ cannot be used in a native entry point signature
*/

// Optional wrappers must not make an input view available through a host uniform binding.
void nativeUniform(uniform Optional<HitInput> input) {}
/*UNIFORM:
                           ^^^^^^^^ invalid native entry point signature
                           ^^^^^^^^ cannot be used in a native entry point signature
*/

struct Packet
{
    Optional<HitInput> values[2];
}

struct OuterPacket
{
    Packet packet;
}

// Ordinary structs must be checked through their instance fields, including array elements.
void nativeField(in OuterPacket input) {}
/*FIELD:
                    ^^^^^^^^^^^ invalid native entry point signature
                    ^^^^^^^^^^^ cannot be used in a native entry point signature
*/

// The buffer's element type describes bound storage, so its nested input view is still invalid.
void nativeResource(uniform ConstantBuffer<OuterPacket> input) {}
/*RESOURCE:
                            ^^^^^^^^^^^^^^ invalid native entry point signature
                            ^^^^^^^^^^^^^^ cannot be used in a native entry point signature
*/

void nativeStructuredResource(uniform StructuredBuffer<OuterPacket> input) {}
/*STRUCTURED_RESOURCE:
                                      ^^^^^^^^^^^^^^^^ invalid native entry point signature
                                      ^^^^^^^^^^^^^^^^ cannot be used in a native entry point signature
*/

Optional<HitInput> nativeResult()
/*RESULT:
^^^^^^^^ invalid native entry point signature
^^^^^^^^ cannot be used in a native entry point signature
*/
{
    return none;
}

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
void nativeGeneric<T : IInputCarrier>(in Optional<T.Item> input) {}
/*GENERIC:
                                         ^^^^^^^^ invalid native entry point signature
                                         ^^^^^^^^ cannot be used in a native entry point signature
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

    // Selecting Stage asks for a structural entry. Selecting Stage.invoke asks for this same
    // function as an ordinary native entry, so only the second selection must be rejected.
    static void invoke(in HitInput input)
/*METHOD:
                          ^^^^^^^^ invalid native entry point signature
                          ^^^^^^^^ cannot be used in a native entry point signature
*/
    {
        forwardInput(input);
    }
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

struct RecursivePayload
{
    Ptr<RecursivePayload> next;
    uint value;
}

// Check that following instance fields terminates for recursive pointer types. This run ignores
// target capability restrictions so that it only tests the signature's structural-input rule.
void nativeRecursive(inout RecursivePayload payload) {}

#ifdef INHERITED
struct BasePayload
{
    Optional<HitInput> input;
}

// Native payload validation must include instance storage inherited from a concrete base.
struct DerivedPayload : BasePayload {}
/*INHERITED:
                        ^^^^^^^^^^^ support for inheritance is unstable
*/

void nativeInherited(inout DerivedPayload payload) {}
/*INHERITED:
                           ^^^^^^^^^^^^^^ invalid native entry point signature
                           ^^^^^^^^^^^^^^ cannot be used in a native entry point signature
*/
#endif
