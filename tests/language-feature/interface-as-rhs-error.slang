//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

// No error messages should show for concrete types above
//CHECK-NOT:: error

// These should produce errors - interface types as RHS
bool testIsOperatorWithInterface<T>()
{
    //CHECK: ([[# @LINE+1]]): error 30301: 'is' and 'as' operators do not support interface types as the right-hand side
    return (T is IMyInterface);
}

void testAsOperatorWithInterface<T>(T value)
{
    //CHECK: ([[# @LINE+1]]): error 30301: 'is' and 'as' operators do not support interface types as the right-hand side
    let result = value as IMyInterface;
}

// No error messages should show for concrete types below
//CHECK-NOT:: error

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
