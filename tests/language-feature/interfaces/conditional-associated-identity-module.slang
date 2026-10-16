//TEST_IGNORE_FILE:

module "conditional-associated-identity-module";

public interface IData { int get(); }

[sealed]
public interface IIdentityValue
{
    associatedtype Value : IData;
}

public interface IMarker : IData {}

public struct Box<T>
{
    // This same-name alias must not determine the interface's associated binding.
    public typealias Value = float;
    public T payload;
}

extension Box<int> : IMarker { int get() { return payload; } }
extension Box<float> : IMarker { int get() { return int(payload); } }

extension<T : IMarker> T : IIdentityValue
{
    typealias Value = T;
}

public struct Descriptor : IIdentityValue
{
    typealias Value = Box<int>;
}

[sealed]
public interface IMixedValue
{
    associatedtype Value;
}

public struct MixedBox<T>
{
    public typealias Value = MixedBox<T>;
    public T payload;
}

extension MixedBox<int> : IMixedValue
{
    typealias Value = MixedBox<int>;
}

extension MixedBox<float> : IMixedValue
{
    typealias Value = float;
}

[sealed]
public interface IClosedValue
{
    associatedtype Value;
}

public interface IOpenValue : IClosedValue {}

public struct EscapeBox<T>
{
    public T payload;
}

extension EscapeBox<float> : IClosedValue
{
    typealias Value = EscapeBox<float>;
}
