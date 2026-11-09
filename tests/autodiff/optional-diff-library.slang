// Library half of optional-differentiability-serialized.slang.
module optional_diff_library;

[Differentiable]
public float serializedSquare<T>(T metadata, float value) where optional T : IDifferentiable
{
    return value * value;
}

public struct SerializedIndexed : IDifferentiableFuncWithDiffThis<float, uint, float>
{
    public typealias Differential = SerializedIndexed;

    [Differentiable]
    public float operator()(uint index, float value)
    {
        return value * value + float(index);
    }
}

int hasDifferential<T>() where optional T : IDifferentiable
{
    if (T is IDifferentiable)
        return 1;
    else
        return 0;
}

public int serializedCount<each T>() where optional T : IDifferentiable
{
    int count = 0;
    expand count += hasDifferential<each T>();
    return count;
}
