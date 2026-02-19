//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

interface IMyInterface
{
    int getValue();
}

struct ConcreteImpl : IMyInterface
{
    int getValue() { return 42; }
}

struct AnotherType
{
    float value;
}

// These should produce errors - interface types as RHS
bool testIsOperatorWithInterface<T>()
{
    return (T is IMyInterface);
//diag:       ^^ cannot use 'is' with interface on right-hand side
//diag:       ^^ cannot use 'is' operator with an interface type as the right-hand side without a corresponding optional constraint. Use a concrete type instead, or add an optional constraint for the interface type.
}

void testAsOperatorWithInterface<T>(T value)
{
    let result = value as IMyInterface;
//diag:                ^^ cannot use 'as' with interface on right-hand side
//diag:                ^^ cannot use 'as' operator with an interface type as the right-hand side. Use a concrete type instead. If you want to use an optional constraint, use an 'if (T is IInterface)' block instead.
}

// These should work - concrete types as RHS
bool testIsOperatorWithConcreteType<T>()
{
    return (T is ConcreteImpl); // Should compile without error
}

void testAsOperatorWithConcreteType<T>(T value)
{
    // Test as operator with concrete types - should compile without error
    let result = value as ConcreteImpl;
}

void main()
{
    ConcreteImpl impl;
    AnotherType other;

    // Test error cases - these should produce the errors checked above
    bool result1 = testIsOperatorWithInterface<ConcreteImpl>();
    testAsOperatorWithInterface<ConcreteImpl>(impl);

    // Test success cases - these should compile without errors
    // If ANY of these had errors, compilation would fail
    bool result2 = testIsOperatorWithConcreteType<ConcreteImpl>();
    testAsOperatorWithConcreteType<ConcreteImpl>(impl);

    // Additional concrete type tests
    bool isTest1 = (impl is ConcreteImpl);
    bool isTest2 = (other is AnotherType);
    bool isTest3 = (ConcreteImpl is ConcreteImpl);
    bool isTest4 = (AnotherType is ConcreteImpl);

    // Test as operator directly
    let asTest1 = impl as AnotherType;
    let asTest2 = other as ConcreteImpl;
    let asTest3 = impl as ConcreteImpl;
}
