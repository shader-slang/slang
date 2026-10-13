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

static MutableValue externalValue;

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

// Inheritance is intentional here because `mutateBase` exercises captured inherited-member
// lookup. Suppress the unstable-inheritance warning so this diagnostic test remains focused on
// receiver mutability.
#pragma warning(disable:30816)

struct BaseValue
{
    [mutating]
    void mutateBase() {}
}

struct DerivedValue : BaseValue
{}

extension float2
{
    [mutating]
    void clear() {}
}

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

    // The method call needs read/write access to the complete property value. Lowering reads that
    // value through the `ref` accessor into a temporary, then writes it back through the setter
    // because a setter takes precedence over a `ref` accessor for a whole-value write. That
    // mutating setter cannot use the captured, immutable `outer` value as its receiver.
    let mutateThroughNonmutatingRef = () =>
        outer.propertyWithLateNonmutatingRef.mutate();
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating method 'set' cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member

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

void checkCapturedSwizzleValueCategories()
{
    float2x2 matrix = {};
    let mutateMatrixSwizzle = () => matrix._m00_m01.clear();
//CHECK:                                                 ^ mutating method cannot be called on immutable value
//CHECK:                                                 ^ mutating method 'clear' cannot be called on an immutable value

    Tuple<MutableValue, MutableValue> tuple = {};
    let mutateTupleElement = () => tuple._0.mutate();
//CHECK:                                          ^ mutating method cannot be called on immutable value
//CHECK:                                          ^ mutating method 'mutate' cannot be called on an immutable value

    mutateMatrixSwizzle();
    mutateTupleElement();
}

void checkCapturedPointerValueCategories()
{
    MutableValue values[2] = {};
    MutableValue* pointer = __getAddress(values[0]);

    // Capturing a pointer by value does not make the storage that it points to immutable.
    let mutateDereferencedValue = () => (*pointer).mutate();
    let mutateIndexedValue = () => pointer[1].mutate();

    mutateDereferencedValue();
    mutateIndexedValue();
}
