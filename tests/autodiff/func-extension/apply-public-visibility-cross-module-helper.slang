//TEST_IGNORE_FILE:

public T identityWithCustomGradient<T : IFloat>(T input)
    where T.Differential == T
{
    return input;
}

public struct IdentityWithCustomGradientContext<T : IFloat>
    where T.Differential == T
{
    public T scale;

    public void operator()(out T inputGradient, T outputGradient)
    {
        inputGradient = outputGradient * scale;
    }
}

public __func_extension<T : IFloat>
__apply(identityWithCustomGradient<T>)(T input)
    -> Tuple<T, IdentityWithCustomGradientContext<T>>
    where T.Differential == T
{
    return makeTuple(input, IdentityWithCustomGradientContext<T>(T(7)));
}
