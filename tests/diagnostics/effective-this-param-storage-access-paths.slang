//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Semantic checking must validate every abstract-storage accessor required to evaluate an
// expression, whether the result is consumed as a value or used as a method receiver. These cases
// distinguish the getter's receiver from storage in the value or pointer returned by that getter.

struct MutableValue
{
    [nonmutating]
    void observe()
    {}

    [mutating]
    void mutate()
    {}
}

MutableValue globalValue;
Ptr<MutableValue> globalPointer;

struct GetterContainer
{
    property int number
    {
        [mutating]
        get { return 0; }
    }

    property MutableValue value
    {
        [mutating]
        get { return globalValue; }
    }

    property Ptr<MutableValue> pointer
    {
        [mutating]
        get { return globalPointer; }
    }

    property MutableValue items[1]
    {
        [nonmutating]
        get { return {}; }

        [nonmutating]
        set {}
    }
}

void warnForMutatingGetterInInferredInitializer(GetterContainer input)
{
    let value = input.number;
//CHECK:              ^^^^^^ mutating method called on `in` parameter
//CHECK:              ^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForMutatingGetterInInferredLambdaReturn()
{
    let read = (GetterContainer input) => input.number;
//CHECK:                                        ^^^^^^ mutating method called on `in` parameter
//CHECK:                                        ^^^^^^ mutating method 'get' called on `in` parameter 'input'
    GetterContainer container = {};
    read(container);
}

void warnForMutatingGetterInExplicitCast(GetterContainer input)
{
    float value = (float)input.number;
//CHECK:                       ^^^^^^ mutating method called on `in` parameter
//CHECK:                       ^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForMutatingGetterInImplicitConversion(GetterContainer input)
{
    float value = input.number;
//CHECK:                ^^^^^^ mutating method called on `in` parameter
//CHECK:                ^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForMutatingGetterInInitializerList(GetterContainer input)
{
    float2 value = { input.number, 0.0 };
//CHECK:                   ^^^^^^ mutating method called on `in` parameter
//CHECK:                   ^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForMutatingGetterInBuiltinOperator(GetterContainer input)
{
    int value = input.number + 1;
//CHECK:              ^^^^^^ mutating method called on `in` parameter
//CHECK:              ^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForMutatingGetterOnInput(GetterContainer input)
{
    // The selected method is nonmutating, but evaluating its receiver invokes the mutating
    // getter. Its receiver mode requires writable access to `input`, so the diagnostic must
    // identify the getter rather than `observe`.
    input.value.observe();
//CHECK:  ^^^^^ mutating method called on `in` parameter
//CHECK:  ^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void rejectMutatingGetterOnCapturedValue()
{
    GetterContainer container = {};
    let call = () => container.value.observe();
//CHECK:                       ^^^^^ mutating method cannot be called on immutable value
//CHECK:                       ^^^^^ mutating method 'get' cannot be called on an immutable value
//CHECK:             ^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void warnForPointerGetterOnInput(GetterContainer input)
{
    // Pointer indexing reads the pointer and then selects pointee storage. Evaluating `pointer`
    // still invokes the mutating getter on `input`, while the selected method requires writable
    // access only to the pointee. The compiler therefore diagnoses only the getter's receiver.
    input.pointer[0].mutate();
//CHECK:  ^^^^^^^ mutating method called on `in` parameter
//CHECK:  ^^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void warnForPointerGetterBeforeDereference(GetterContainer input)
{
    // Explicit dereference, like pointer indexing, reads the pointer expression but selects the
    // pointee as the method receiver storage.
    (*input.pointer).mutate();
//CHECK:    ^^^^^^^ mutating method called on `in` parameter
//CHECK:    ^^^^^^^ mutating method 'get' called on `in` parameter 'input'
}

void rejectIndexedProjectionWithoutRef(GetterContainer container)
{
    // Indexing needs an address for the property value before it can select an element. A
    // getter/setter pair provides whole-value read and write access, but no address, so checking
    // rejects the projection at `items` rather than treating the later method receiver as storage.
    container.items[0].mutate();
//CHECK:      ^^^^^ property or subscript does not provide an address
//CHECK:      ^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

struct IndexedFieldStorage
{
    int values[1];

    property int index
    {
        get { return 0; }
    }
}

void acceptReadOnlyAbstractStorageInIndex(inout IndexedFieldStorage storage)
{
    // The assignment use belongs to the indexed field, while evaluating the index only reads its
    // getter. This case verifies that `CheckSimpleSubscriptExpr()` attaches the enclosing write to
    // the indexed-field projection, not to the index expression evaluated last.
    storage.values[storage.index] = 1;
}

struct GetSetProjectionStorage
{
    property int4 vectorValue
    {
        [nonmutating]
        get { return int4(0); }

        [nonmutating]
        set {}
    }

    property Tuple<int, int> tupleValue
    {
        [nonmutating]
        get { return makeTuple(0, 0); }

        [nonmutating]
        set {}
    }
}

void incrementProjectedValue(inout int value)
{
    value++;
}

void rejectDirectAddressProjections(GetSetProjectionStorage storage, int index)
{
    // A dynamic index into a swizzle needs an address for `vectorValue`. Whole-value property
    // writeback cannot provide one, so checking rejects the property access before the argument.
    incrementProjectedValue(storage.vectorValue.xy[index]);
//CHECK:                            ^^^^^^^^^^^ property or subscript does not provide an address
//CHECK:                            ^^^^^^^^^^^ cannot take a direct reference to this property because it has no `ref` accessor

    // Selecting a tuple element has the same direct-address requirement.
    incrementProjectedValue(storage.tupleValue._0);
//CHECK:                            ^^^^^^^^^^ property or subscript does not provide an address
//CHECK:                            ^^^^^^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

struct NestedRefStorage
{
    int backing;

    property int value
    {
        [mutating]
        ref { return backing; }
    }
}

struct NestedStorageContainer
{
    NestedRefStorage backing;

    property NestedRefStorage value
    {
        [nonmutating]
        get { return backing; }

        [mutating]
        set { backing = newValue; }
    }
}

void rejectRefAccessorOnWritebackReceiver(NestedStorageContainer storage)
{
    // The inner `ref` accessor may return an address into its receiver. The outer property can
    // produce only a temporary copy followed by setter write-back, which would commit that copy
    // before `incrementProjectedValue` uses the returned address. Reject the outer property
    // because it cannot supply the stable address required by the inner accessor.
    incrementProjectedValue(storage.value.value);
//CHECK:                            ^^^^^ property or subscript does not provide an address
//CHECK:                            ^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

