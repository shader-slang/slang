//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A type-equality cast is transparent to abstract-storage access. A whole-value `inout` operation
// must retain the enclosing property's getter/setter choice across that cast.

struct MutableValue
{
    int value;
}

// A module-scope variable without `static` is an immutable shader parameter. This backing value is
// explicitly static because the test needs addressable storage for both `ref` accessors.
static MutableValue referencedValue;

struct Holder
{
    property MutableValue mutatingSetterMustBeSelected
    {
        [nonmutating] get { return {}; }
        [mutating] set {}
        [nonmutating] ref { return referencedValue; }
    }

    property MutableValue nonmutatingSetterMustBeSelected
    {
        [nonmutating] get { return {}; }
        [nonmutating] set {}
        [mutating] ref { return referencedValue; }
    }
}

void mutate<T>(inout T value) {}

void exercise<T>(Holder input)
    where T == MutableValue
{
    let holder = input;

    // A type-equality cast is transparent to lowering. Whole-value `inout` access therefore uses
    // the getter and mutating setter rather than the nonmutating `ref` accessor.
    mutate<T>(holder.mutatingSetterMustBeSelected);
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating method 'set' cannot be called on an immutable value

    // The opposite accessor modes compile because whole-value `inout` access still selects the
    // nonmutating getter/setter pair instead of the mutating `ref` accessor.
    mutate<T>(holder.nonmutatingSetterMustBeSelected);
}

void instantiate(Holder input)
{
    exercise<MutableValue>(input);
}

