//TEST:SIMPLE(filecheck=CHECK):

interface MyInterface
{
    associatedtype CompatibilityClass;

    __generic <Other : MyInterface>
    This f(Other other) where Other::CompatibilityClass == This::CompatibilityClass;

    __generic <Other : MyInterface>
    This g(Other other) where This::CompatibilityClass == Other::CompatibilityClass;
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
};

__generic <T : MyInterface>
void test(T t)
    where int == T::CompatibilityClass
{}

// CHECK-NOT: 30402
// CHECK-NOT: 30404
// CHECK-NOT: 30405