//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// After a lambda captures an outer local by value, every expression derived from that capture must
// be treated as immutable. These cases cover members, indices, parentheses, swizzles,
// `[nonmutating]` setters, and `ref` accessors.

struct MutableValue
{
    [mutating]
    void setValue(int newValue)
    {
        value = newValue;
    }

    property int ordinaryProperty
    {
        get { return value; }
        set { value = newValue; }
    }

    property int nonmutatingProperty
    {
        get { return value; }
        [nonmutating] set {}
    }

    int value;
};

MutableValue referencedValue;

void mutateInOut(inout MutableValue value)
{
    value.setValue(99);
}

void mutateRef(__ref MutableValue value)
{
    value.setValue(99);
}

struct OuterValue
{
    MutableValue inner;
    MutableValue items[1];

    __subscript(int index) -> int
    {
        get { return items[index].value; }
        set { items[index].value = newValue; }
    }

    __subscript(float index) -> int
    {
        get { return inner.value; }
        [nonmutating] set {}
    }

    __subscript(uint index) -> MutableValue
    {
        set {}
        [nonmutating]
        ref { return referencedValue; }
    }

    __subscript(int64_t index) -> MutableValue
    {
        [nonmutating] set {}
        [mutating]
        ref { return referencedValue; }
    }

    property MutableValue mixedProperty
    {
        set {}
        [nonmutating]
        ref { return referencedValue; }
    }

    property MutableValue refPreferredProperty
    {
        [nonmutating] set {}
        [mutating]
        ref { return referencedValue; }
    }

    property MutableValue getterAndSetterPreferredProperty
    {
        [nonmutating] get { return {}; }
        [nonmutating] set {}
        [mutating]
        ref { return referencedValue; }
    }

    property Ptr<int, Access::ReadWrite> pointerProperty
    {
        [nonmutating] get { return __getAddress(referencedValue.value); }
    }

    property MutableValue getterOnlyProperty
    {
        get { return {}; }
    }
};

extension float2
{
    [mutating]
    void clear() {}
}

void exerciseCapturedMemberAndIndexExpressions()
{
    OuterValue outer = {};

    let mutateField = () => outer.inner.setValue(1);
//CHECK:                                        ^ mutating method cannot be called on immutable value
//CHECK:                                        ^ mutating method 'setValue' cannot be called on an immutable value
//CHECK:                          ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

    let mutateElement = () => outer.items[0].setValue(2);
//CHECK:                                             ^ mutating method cannot be called on immutable value
//CHECK:                                             ^ mutating method 'setValue' cannot be called on an immutable value
//CHECK:                                 ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

    mutateField();
    mutateElement();
}

void exerciseCapturedParenthesizedValue()
{
    MutableValue outer = {};
    let call = () => (outer).setValue(1);
//CHECK:                             ^ mutating method cannot be called on immutable value
//CHECK:                             ^ mutating method 'setValue' cannot be called on an immutable value
//CHECK:             ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void exerciseCapturedSwizzle(inout float4 value)
{
    let call = () => value.xy.clear();
//CHECK:                           ^ mutating method cannot be called on immutable value
//CHECK:                           ^ mutating method 'clear' cannot be called on an immutable value
//CHECK:                   ^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void exerciseCapturedAssignments()
{
    OuterValue outer = {};
    let assign = () =>
    {
        outer.inner.value = 1;
//CHECK:            ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:                  ^ left of '=' is not an l-value
//CHECK:                  ^ left of '=' is not an l-value.
        outer.inner.ordinaryProperty = 2;
//CHECK:                             ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                             ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:      ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // A non-mutating setter accepts the captured receiver without any diagnostic.
        outer.inner.nonmutatingProperty = 3;

        // The setter requires writable receiver storage. A whole-value method call or `inout`
        // argument therefore cannot use the captured receiver, even though the `[nonmutating]`
        // `ref` accessor makes projected writes and `ref` arguments valid.
        outer.mixedProperty.setValue(4);
//CHECK:                            ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                            ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        mutateInOut(outer.mixedProperty);
//CHECK:                  ^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                  ^^^^^^^^^^^^^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:            ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        mutateRef(outer.mixedProperty);
        outer.mixedProperty.value = 5;

        // Whole-property assignment selects the setter rather than the `ref` accessor, so the
        // immutable captured receiver is rejected.
        outer.mixedProperty = {};
//CHECK:                    ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                    ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // A projected write prefers the `ref` accessor. The mutating `ref` accessor cannot use the
        // captured receiver even though whole-property assignment could use the non-mutating
        // setter.
        outer.refPreferredProperty.value = 6;
//CHECK:                                 ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                 ^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // With no getter, a whole-value read falls back to the mutating `ref` accessor and is
        // rejected. A `ref` argument also requires that accessor rather than the setter.
        outer.refPreferredProperty.setValue(7);
//CHECK:                                   ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                   ^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        mutateInOut(outer.refPreferredProperty);
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:            ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        mutateRef(outer.refPreferredProperty);
//CHECK:                ^^^^^^^^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                ^^^^^^^^^^^^^^^^^^^^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:          ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // When a getter is present, whole-value method and `inout` operations use the getter and
        // setter instead of the `ref` accessor. A projected write and a `ref` argument still prefer
        // the mutating `ref` accessor and are rejected.
        outer.getterAndSetterPreferredProperty.setValue(8);
        mutateInOut(outer.getterAndSetterPreferredProperty);
        outer.getterAndSetterPreferredProperty.value = 9;
//CHECK:                                             ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                             ^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        mutateRef(outer.getterAndSetterPreferredProperty);
//CHECK:                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:          ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // A getter can materialize the outer property value. The inner non-mutating setter does
        // not require that temporary, or the captured `outer`, to be writable.
        outer.getterOnlyProperty.nonmutatingProperty = 10;

        // Pointer indexing denotes pointee storage. The captured container and the getter-only
        // pointer property can be immutable while the pointed-to integer remains writable.
        outer.pointerProperty[0] = 11;
    };
    assign();
}

void exerciseCapturedSubscriptAssignments()
{
    OuterValue outer = {};
    let assign = () =>
    {
        outer[0] = 1;
//CHECK:         ^ mutating storage accessor cannot be called on an immutable value
//CHECK:         ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // The `float` overload has a `[nonmutating]` setter and remains assignable after capture.
        outer[0.0] = 2;

        // As with the mixed property, the `ref` accessor supports nested mutation, while
        // whole-subscript assignment selects the setter and rejects the immutable receiver.
        outer[uint(0)].setValue(3);
//CHECK:                       ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                       ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        outer[uint(0)].value = 4;
        outer[uint(0)] = {};
//CHECK:               ^ mutating storage accessor cannot be called on an immutable value
//CHECK:               ^ mutating 'set' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

        // Projected subscript writes prefer the `ref` accessor over the non-mutating setter.
        outer[int64_t(0)].value = 5;
//CHECK:                        ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                        ^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    };
    assign();
}

void exerciseCapturedMatrixSwizzle()
{
    float2x2 matrix = {};
    let assign = () => matrix._m00 = 1.0;
//CHECK:                      ^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:                           ^ left of '=' is not an l-value
//CHECK:                           ^ left of '=' is not an l-value.
    assign();
}
