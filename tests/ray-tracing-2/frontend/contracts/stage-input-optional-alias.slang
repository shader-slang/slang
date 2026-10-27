//DIAGNOSTIC_TEST:SIMPLE(diag=DECLARATION): -experimental-feature -ignore-capabilities -no-codegen -DTEST_DECLARATION
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -experimental-feature -ignore-capabilities -no-codegen -DTEST_NESTED
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_CONTEXT): -experimental-feature -ignore-capabilities -no-codegen -DTEST_GENERIC_CONTEXT
//DIAGNOSTIC_TEST:SIMPLE(diag=CAST): -experimental-feature -ignore-capabilities -no-codegen -DTEST_CAST
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTRUCTOR): -experimental-feature -ignore-capabilities -no-codegen -DTEST_CONSTRUCTOR
//DIAGNOSTIC_TEST:SIMPLE(diag=NONE): -experimental-feature -ignore-capabilities -no-codegen -DTEST_NONE

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Context : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = uint;
}

typealias Input = rt::MissInput<Context>;

#if defined(TEST_DECLARATION)
// Even wrapping an existing input is invalid: stage inputs are opaque, compiler-supplied values.
void wrapExistingInput(in Input input)
{
    Optional<Input> maybe = input;
/*DECLARATION:
             ^^^^^ ray-tracing stage input cannot be used as a generic type argument
             ^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' cannot be used as a generic type argument
*/
}
#elif defined(TEST_NESTED)
Optional<Optional<Input>> nested;
/*NESTED:
                  ^^^^^ ray-tracing stage input cannot be used as a generic type argument
                  ^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' cannot be used as a generic type argument
*/
#elif defined(TEST_GENERIC_CONTEXT)
// The context may remain generic, but this is already known to be a stage input type.
void wrapGenericContext<C : rt::IPayloadContext>(in rt::MissInput<C> input)
{
    Optional<rt::MissInput<C>> maybe = input;
/*GENERIC_CONTEXT:
                 ^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
                 ^^^^^^^^^ cannot be used as a generic type argument
*/
}
#elif defined(TEST_CAST)
void castInput(in Input input)
{
    // `as` produces Optional<Input>, so it follows the ordinary opaque-type restriction too.
    let maybe = input as Input;
/*CAST:
                      ^^ 'Optional<T>' cannot wrap a resource or opaque type
                      ^^ 'rt.MissInput<Context>' is a resource or opaque type and cannot be used as the value type of 'Optional<T>'.
*/
}
#else
interface IInputCarrier
{
    associatedtype Item;
}

struct InputCarrier : IInputCarrier
{
    typealias Item = Input;
}

#if defined(TEST_CONSTRUCTOR)
typealias Maybe<T : IInputCarrier> = Optional<T.Item>;

void aliasConstructor(in Input input)
{
    // The alias hides Input while declaring Maybe, but the constructor sees its concrete type.
    let maybe = Maybe<InputCarrier>(input);
/*CONSTRUCTOR:
                                    ^^^^^ ray-tracing stage input cannot be used as a generic type argument
                                    ^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' cannot be used as a generic type argument
*/
}
#else
void consume<T : IInputCarrier>(Optional<T.Item> input) {}

void createAbsentInput()
{
    // T.Item was unknown in consume's declaration; this call resolves it to Input.
    consume<InputCarrier>(none);
/*NONE:
                          ^^^^ 'Optional<T>' cannot wrap a resource or opaque type
                          ^^^^ 'rt.MissInput<Context>' is a resource or opaque type and cannot be used as the value type of 'Optional<T>'.
*/
}
#endif
#endif
