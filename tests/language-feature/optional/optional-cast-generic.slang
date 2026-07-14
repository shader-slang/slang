//TEST:INTERPRET(filecheck=CHECK):

interface IValue
{
    int get();
}

struct ConcreteValue : IValue
{
    int x;
    int get() { return x; }
}

// Generic function that returns Optional<IValue> from Optional<T : IValue>
__generic<T : IValue>
Optional<IValue> wrap(Optional<T> v)
{
    return v; // implicit coercion: Optional<T> -> Optional<IValue>
}

void main()
{
    // Test coercion with a value — function call works for the some case
    ConcreteValue cv = { 42 };
    Optional<ConcreteValue> optCv = cv;
    Optional<IValue> iv = wrap(optCv);

    // CHECK: 42
    if (iv.hasValue)
        printf("%d\n", iv.value.get());

    // Test none path inline — avoid the pre-existing interpreter limitation
    // with Optional<interface> returned from conditional-return functions
    Optional<ConcreteValue> empty = none;
    Optional<IValue> emptyIV = empty; // inline implicit coercion

    // CHECK: empty
    if (!emptyIV.hasValue)
        printf("empty\n");
}
