//TEST(smoke):SIMPLE:

interface IModel
{}

struct Leaf : IModel
{}

struct Node<T : IModel> : IModel
{}

typealias T0 = Leaf;
typealias T1 = Node<T0>;
typealias T2 = Node<T1>;
typealias T3 = Node<T2>;
typealias T4 = Node<T3>;
typealias T5 = Node<T4>;
typealias T6 = Node<T5>;
typealias T7 = Node<T6>;
typealias T8 = Node<T7>;
typealias T9 = Node<T8>;
typealias T10 = Node<T9>;
typealias T11 = Node<T10>;
typealias T12 = Node<T11>;

void inferModel<T : IModel>(T model)
{}

void consume<U>(T12 model, U value)
{}

void exercise(T12 model)
{
    // Infer the entire constrained tree as a generic argument.
    inferModel(model);

    // Inferring U also substitutes a generic decl-ref into the already-deep T12 parameter type.
    consume(model, 0);
}
