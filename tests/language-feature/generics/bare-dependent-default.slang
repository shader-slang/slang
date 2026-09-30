//TEST:INTERPRET(filecheck=CHECK):

interface IWrapper<T>
{
    T getValue();
}

struct Wrapper<T> : IWrapper<T>
{
    T value;
    T getValue() { return value; }
}

struct View<T = uint, W : IWrapper<T> = Wrapper<T>>
{
    W wrapped;
    T getValue() { return wrapped.getValue(); }
}

struct Values<let N : int = 2, let M : int = N + 1>
{
    int[M] values;
}

void main()
{
    // Omitting the entire argument list must substitute earlier defaults into
    // later defaults and constraints, just as an explicit application does.
    View view;
    view.wrapped.value = 42;
    printf("%u\n", view.getValue());
    // CHECK: 42

    Values values;
    printf("%d\n", values.values.getCount());
    // CHECK: 3
}
