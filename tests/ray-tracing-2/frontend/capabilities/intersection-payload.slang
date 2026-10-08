//TEST:SIMPLE: -experimental-feature -entry AccessPayload -stage intersection -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -entry AccessPayload -stage intersection -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=UNSUPPORTED): -experimental-feature -entry AccessPayload -stage intersection -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=UNSUPPORTED): -experimental-feature -entry AccessPayload -stage intersection -target glsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=UNSUPPORTED): -experimental-feature -entry AccessPayload -stage intersection -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ReadRecord -stage intersection -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ReadRecord -stage intersection -target glsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ReadRecord -stage intersection -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ReadRecord -stage intersection -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ReadRecord -stage intersection -target metal -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GuardedPayload -stage intersection -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GuardedPayload -stage intersection -target glsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GuardedPayload -stage intersection -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GuardedPayload -stage intersection -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -entry GuardedPayload -stage intersection -target metal -no-codegen

// Payload access is an additional target capability, not a restriction on IntersectionInput.
// These checks cover the source contract; native adapters are outside this frontend slice.
import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = uint;
}

void replacePayload<P>(inout P destination, P replacement)
{
    destination = replacement;
}

// The context need not be concrete to read, assign, or pass the payload by mutable reference.
void accessPayload<C : rt::IHitContext>(in rt::IntersectionInput<C> input)
{
    C.Payload saved = input.payload;
    input.payload = saved;
    replacePayload(input.payload, saved);
}

struct AccessPayload : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        accessPayload(input);
        input.payload.value += 1;
    }
}

// A nonempty payload type in the context does not make ordinary intersection inputs target-specific.
struct ReadRecord : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        uint record = input.record;
    }
}

// Ordinary target selection can keep a shader portable while allowing native payload access.
struct GuardedPayload : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        __target_switch
        {
        case cuda:
        case metal:
            accessPayload(input);
        default:
            uint record = input.record;
        }
    }
}

//UNSUPPORTED: unavailable features in entry point
