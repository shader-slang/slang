//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

extension<T> Ptr<T>
{
    __subscript(float index) -> T
    {
        [__ref]
        ref { return this[0]; }
    }

    __subscript(double index) -> T
    {
        [nonmutating] get { return this[0]; }
        [mutating] set {}
    }
}

Ptr<int> makePointer();

struct MutableValue
{
    [mutating] void mutate() {}
};

void rejectRefAccessorOnRValuePointer()
{
    makePointer()[0.0] = 1;
//CHECK:               ^ mutating storage accessor cannot be called on an immutable value
//CHECK:               ^ mutating 'ref' accessor cannot be called on an immutable value
}

void warnForMutationOfInputPointer(Ptr<MutableValue> input)
{
    input[double(0)].mutate();
//CHECK:                   ^ mutating method called on `in` parameter
//CHECK:                   ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}
