public interface IValueProvider
{
    uint getValue();
}

[shader("compute")]
[numthreads(1, 1, 1)]
public void writeImportedValue<T : IValueProvider>(
    RWStructuredBuffer<uint> output,
    T value)
{
    output[0] = value.getValue();
}
