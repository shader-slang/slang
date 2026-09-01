//TEST(smoke):SIMPLE:

interface IModel
{
    void consume(This value)
    {}
}

struct Leaf : IModel
{}

struct Pair<First : IModel, Second : IModel> : IModel
{}

typealias T0 = Leaf;
typealias T1 = Pair<Leaf, T0>;
typealias T2 = Pair<Leaf, T1>;
typealias T3 = Pair<Leaf, T2>;
typealias T4 = Pair<Leaf, T3>;
typealias T5 = Pair<Leaf, T4>;
typealias T6 = Pair<Leaf, T5>;
typealias T7 = Pair<Leaf, T6>;
typealias T8 = Pair<Leaf, T7>;
typealias T9 = Pair<Leaf, T8>;
typealias T10 = Pair<Leaf, T9>;
typealias T11 = Pair<Leaf, T10>;
typealias T12 = Pair<Leaf, T11>;
typealias T13 = Pair<Leaf, T12>;
typealias T14 = Pair<Leaf, T13>;
typealias T15 = Pair<Leaf, T14>;
typealias T16 = Pair<Leaf, T15>;

void consumePair<T : IModel>(Pair<T, T> value)
{}

T16 value;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{}
