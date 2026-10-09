//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

struct MutableValue
{
    int value;
}

MutableValue referencedValue;

struct Holder
{
    property MutableValue setterPreferred
    {
        set {}
        [nonmutating] ref { return referencedValue; }
    }

    property MutableValue refMustNotBePreferred
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
    // the setter, not the nonmutating `ref` accessor.
    mutate<T>(holder.setterPreferred);
//CHECK:             ^^^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:             ^^^^^^^^^^^^^^^ mutating 'set' accessor cannot be called on an immutable value

    // Whole-value `inout` access prefers a getter/setter pair over a `ref` accessor.
    mutate<T>(holder.refMustNotBePreferred);
}

void instantiate(Holder input)
{
    exercise<MutableValue>(input);
}
