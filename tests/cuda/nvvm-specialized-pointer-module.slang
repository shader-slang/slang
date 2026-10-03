//TEST_IGNORE_FILE:
// Imported generic bodies must specialize pointer arguments by canonical type identity.
[noinline]
public T preserve<T>(T value)
{
    return value;
}

public struct Wrapper<T>
{
    public T value;
    [noinline]
    public T getValue() { return value; }
}
