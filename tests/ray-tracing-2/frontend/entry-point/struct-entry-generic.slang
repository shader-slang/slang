//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CODEGEN): -experimental-feature -entry M<P> -stage miss -target hlsl

import slang.raytracing;

struct P {}
struct R {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext<PayloadType> : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadType;
    typealias Record = R;
}

// Selecting this closed specialization must preserve `P` on the `invoke` declaration reference.
// Reducing the conformance witness to its bare `FuncDecl` leaves `PayloadType` unbound and causes
// AST-to-IR lowering to reject the free generic type. The no-codegen run covers that selection.
// The HLSL run intentionally reaches the unlowered-adapter diagnostic, proving that linking moved
// the specialization-specific structural metadata from the `IRSpecialize` to the resulting
// `IRFunc` instead of losing it or attaching it to the shared generic body.
struct M<PayloadType> : rt::IMissShader
{
    typealias Context = MissContext<PayloadType>;

    void invoke(in rt::MissInput<Context> input)
/*CODEGEN:
         ^^^^^^ structural ray-tracing entry point was not lowered
         ^^^^^^ structural ray-tracing entry point 'M.invoke' must be converted to a native adapter before target code generation
*/
    {}
}
