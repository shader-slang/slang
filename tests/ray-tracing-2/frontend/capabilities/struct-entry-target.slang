//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL,non-exhaustive): -experimental-feature -entry MetalOnlyMiss -stage miss -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry MetalOnlyMiss -stage miss -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_HLSL,non-exhaustive): -experimental-feature -entry GenericMiss<MetalContext> -stage miss -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_HLSL,non-exhaustive): -experimental-feature -entry GenericMiss<MetalContext>.invoke -stage miss -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GenericMiss<MissContext> -stage miss -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GenericMiss<MetalContext> -stage miss -target metal -no-codegen

import slang.raytracing.portable;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Payload {}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

[require(metal)]
void useMetalOnlyFeature() {}

struct MetalOnlyMiss : rt::IMissShader
{
    typealias Context = MissContext;

    static void invoke(in rt::MissInput<Context> input)
/*HLSL:
                ^^^^^^ unavailable features in entry point
                ^^^^^^ entrypoint 'invoke' uses features that are not available in '_miss' stage for 'hlsl' compilation target.
*/
    {
        useMetalOnlyFeature();
//HLSL: ^^^^^^^^^^^^^^^^^^^ see using of 'useMetalOnlyFeature'
    }
}

[require(metal)]
struct MetalContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

// The selected context must contribute its requirements even when invoke never uses input.
struct GenericMiss<C : rt::IPayloadContext> : rt::IMissShader
{
    typealias Context = C;

    static void invoke(in rt::MissInput<Context> input)
/*GENERIC_HLSL:
                ^^^^^^ unavailable features in entry point
                ^^^^^^ entrypoint 'invoke' uses features that are not available in '_miss' stage for 'hlsl' compilation target.
*/
    {}
}
