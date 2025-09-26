//TEST:SIMPLE(filecheck=CHECK):
//TEST:INTERPRET(filecheck=ICHECK):

interface MyInterface
{
    associatedtype CompatibilityClass;

    __generic <Other : MyInterface>
    This f(Other other) where Other::CompatibilityClass == This::CompatibilityClass;

    __generic <Other : MyInterface>
    This g(Other other) where This::CompatibilityClass == Other::CompatibilityClass;

    CompatibilityClass toCompat();
};

struct MyCompatibilityClass {};

__generic <T>
struct MyStruct : MyInterface
{
    typealias CompatibilityClass = MyCompatibilityClass;

    __generic <Other : MyInterface>
    This f(Other other) where CompatibilityClass == Other::CompatibilityClass
    {
        return this;
    }

    __generic <Other : MyInterface>
    This g(Other other) where MyCompatibilityClass == Other::CompatibilityClass
    {
        return this;
    }
    
    CompatibilityClass toCompat()
    {
        return MyCompatibilityClass();
    }
};

struct TestInt : MyInterface
{
    typealias CompatibilityClass = int;
    int value;

    __init(int v)
    {
        value = v;
    }

    __generic <Other : MyInterface>
    This f(Other other) where CompatibilityClass == Other::CompatibilityClass
    {
        return this;
    }

    __generic <Other : MyInterface>
    This g(Other other) where int == Other::CompatibilityClass
    {
        return this;
    }

    int toCompat()
    {
        return value;
    }
}

__generic <T : MyInterface>
void test(T t)
    where int == T::CompatibilityClass
{
    printf("Success x %d!", t.toCompat());
}

void main()
{
    TestInt t = TestInt(12);
    test(t);
}

// CHECK-NOT: 30402
// CHECK-NOT: 30404
// CHECK-NOT: 30405

// ICHECK: Success x 12!