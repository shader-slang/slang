//TEST:SIMPLE: -experimental-feature -ignore-capabilities -no-codegen

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
interface IMarker {}
extension Input : IMarker {}

void useInput(in Input input)
{
    // The copied view still refers to this invocation's payload and record.
    let copy = input;
    copy.payload += copy.record;
}

void useOptional(Optional<Input> maybe)
{
    if (maybe.hasValue)
        useInput(maybe.value);
}

T identity<T>(T value) { return value; }

void wrapExistingInput(in Input input)
{
    // Empty optionals do not create an input. Populated optionals copy an existing view.
    Optional<Input> absent = none;
    Optional<Input> defaultAbsent = Optional<Input>();
    Optional<Input> implicitCopy = input;
    Optional<Input> explicitCopy = Optional<Input>(input);
    let copiedOptional = identity(explicitCopy);
    useOptional(absent);
    useOptional(defaultAbsent);
    useOptional(implicitCopy);
    useOptional(copiedOptional);

    Optional<Optional<Input>> nested = explicitCopy;
    if (nested.hasValue)
        useOptional(nested.value);
}

void recoverExistingInput(IMarker other, in Input input)
{
    // A failed cast has no input. A successful cast preserves the existing view.
    let fromInterface = other as Input;
    useOptional(fromInterface);
    useOptional(input as Input);

    IMarker boxed = input;
    useOptional(boxed as Input);
}

void wrapGenericContext<C : rt::IPayloadContext>(in rt::MissInput<C> input)
{
    Optional<rt::MissInput<C>> maybe = input;
    if (maybe.hasValue)
    {
        let copy = maybe.value;
        Optional<rt::MissInput<C>> fromCast = copy as rt::MissInput<C>;
    }
}

interface IValueType { associatedtype Value; }
struct InputValue : IValueType { typealias Value = Input; }

struct OptionalProvider<T : IValueType>
{
    // These declarations know only T.Value. Member lookup substitutes Input at the use below.
    property Optional<T.Value> maybe { get { return none; } }
    property Optional<T.Value>[1] values { get { return {none}; } }
    property Ptr<Optional<T.Value>> pointer { get { return nullptr; } }
    Optional<T.Value> getValue() { return none; }
}

struct OptionalStorage<T : IValueType>
{
    static Optional<T.Value> stored;
}

void useSpecializedOptional()
{
    let provider = OptionalProvider<InputValue>();
    useOptional(provider.maybe);
    useOptional(provider.getValue());
    useOptional(provider.values[0]);
    useOptional(OptionalStorage<InputValue>.stored);

    // Member access through this pointer implicitly dereferences Optional<Input>.
    let pointer = provider.pointer;
    if (pointer != nullptr && pointer.hasValue)
        useInput(pointer.value);
}
