//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A lambda captures an outer local by value. Receiver validation must use the value category after
// that capture rewrite, including for expressions derived from the captured closure field.

struct MutableValue
{
    int value;

    [mutating]
    void mutate()
    {
        value++;
    }
}

MutableValue externalValue;

struct OuterValue
{
    MutableValue field;
    MutableValue elements[1];

    property MutableValue propertyWithLateNonmutatingRef
    {
        set {}

        [nonmutating]
        ref { return externalValue; }
    }
}

#pragma warning(disable:30816)

struct BaseValue
{
    [mutating]
    void mutateBase() {}
}

struct DerivedValue : BaseValue
{}

void checkCapturedReceiverValueCategories()
{
    OuterValue outer = {};

    let mutateField = () => outer.field.mutate();
//CHECK:                                      ^ mutating method cannot be called on immutable value
//CHECK:                                      ^ mutating method 'mutate' cannot be called on an immutable value
//CHECK:                          ^^^^^ attempting to assign to a const variable or immutable member

    let mutateElement = () => outer.elements[0].mutate();
//CHECK:                                              ^ mutating method cannot be called on immutable value
//CHECK:                                              ^ mutating method 'mutate' cannot be called on an immutable value
//CHECK:                                    ^ attempting to assign to a const variable or immutable member

    let mutateParenthesizedField = () => (outer.field).mutate();
//CHECK:                                                     ^ mutating method cannot be called on immutable value
//CHECK:                                                     ^ mutating method 'mutate' cannot be called on an immutable value

    // The ordinary setter requires writable receiver storage, but the later `ref` accessor does
    // not. The property therefore remains a valid source of writable storage after capture.
    let mutateThroughNonmutatingRef = () =>
        outer.propertyWithLateNonmutatingRef.mutate();

    DerivedValue derived;
    let mutateInheritedField = () => derived.mutateBase();
//CHECK:                                               ^ mutating method cannot be called on immutable value
//CHECK:                                               ^ mutating method 'mutateBase' cannot be called on an immutable value

    mutateField();
    mutateElement();
    mutateParenthesizedField();
    mutateThroughNonmutatingRef();
    mutateInheritedField();
}
