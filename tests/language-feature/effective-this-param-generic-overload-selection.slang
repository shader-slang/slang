//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/effective-this-param-generic-overload-selection.slang -no-codegen

// Explicit generic arguments leave both overloads viable until the value argument selects one;
// the losing mutating overload must not produce an immutable-receiver diagnostic.

struct GenericReceiverSelection
{
    [mutating]
    void selectGeneric<T>(int value) {}

    [nonmutating]
    void selectGeneric<T>(float value) {}

    [nonmutating]
    void useFloatSelectedOverload()
    {
        selectGeneric<int>(1.0);
    }
};
