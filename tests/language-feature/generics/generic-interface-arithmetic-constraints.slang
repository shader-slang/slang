
interface IModule<T, int N, int M>
{
    T eval(T x);
}


float foo<
    int N,
    Decoder : IModule<half, N + 6, 3>>
    (Decoder decoder, no_diff float3 wi )
{
    return wi.x;
}


void bar<
    int N,
    Encoder : IModule<half, 3, N>,
    Decoder : IModule<half, N + 6, 3>>
    (Encoder encoder, Decoder decoder, no_diff float3 wi)
{
    foo(decoder, wi);
}