//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Evaluating a storage expression must validate every abstract-storage accessor it invokes,
// whether the result is consumed as a value or used as a method receiver. These cases distinguish
// the getter's receiver from storage in the value or pointer returned by that getter.

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
//CHECK:              ^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:              ^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForMutatingGetterInInferredLambdaReturn()
{
    let read = (GetterContainer input) => input.number;
//CHECK:                                        ^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:                                        ^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
    GetterContainer container = {};
    read(container);
}

void warnForMutatingGetterInExplicitCast(GetterContainer input)
{
    float value = (float)input.number;
//CHECK:                       ^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:                       ^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForMutatingGetterInImplicitConversion(GetterContainer input)
{
    float value = input.number;
//CHECK:                ^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:                ^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForMutatingGetterInInitializerList(GetterContainer input)
{
    float2 value = { input.number, 0.0 };
//CHECK:                   ^ mutating storage accessor called on `in` parameter
//CHECK:                   ^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForMutatingGetterInBuiltinOperator(GetterContainer input)
{
    int value = input.number + 1;
//CHECK:              ^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:              ^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForMutatingGetterOnInput(GetterContainer input)
{
    // The selected method is nonmutating, but evaluating its receiver invokes the mutating
    // getter. Its receiver mode requires writable access to `input`, so the diagnostic must
    // identify the getter rather than `observe`.
    input.value.observe();
//CHECK:               ^ mutating storage accessor called on `in` parameter
//CHECK:               ^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void rejectMutatingGetterOnCapturedValue()
{
    GetterContainer container = {};
    let call = () => container.value.observe();
//CHECK:                                    ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                    ^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:             ^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void warnForPointerGetterOnInput(GetterContainer input)
{
    // Pointer indexing reads the pointer and then selects pointee storage. Evaluating `pointer`
    // still invokes the mutating getter on `input`, while the selected method requires writable
    // access only to the pointee. The compiler therefore diagnoses only the getter's receiver.
    input.pointer[0].mutate();
//CHECK:                   ^ mutating storage accessor called on `in` parameter
//CHECK:                   ^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void warnForPointerGetterBeforeDereference(GetterContainer input)
{
    // Explicit dereference has the same storage boundary as pointer indexing.
    (*input.pointer).mutate();
//CHECK:    ^^^^^^^ mutating storage accessor called on `in` parameter
//CHECK:    ^^^^^^^ mutating 'get' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void rejectPointerGetterOnCapturedValue()
{
    GetterContainer container = {};
    let call = () => container.pointer[0].mutate();
//CHECK:                                        ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                        ^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:             ^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void rejectPointerGetterBeforeCapturedDereference()
{
    GetterContainer container = {};
    let call = () => (*container.pointer).mutate();
//CHECK:                         ^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                         ^^^^^^^ mutating 'get' accessor cannot be called on an immutable value
//CHECK:               ^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    call();
}

void rejectIndexedProjectionWithoutRef(GetterContainer container)
{
    // Lowering has no writeback representation for an index into a property result. A
    // getter/setter pair therefore cannot provide writable receiver storage without a `ref`
    // accessor on `items`, so semantic checking rejects the method call before lowering.
    container.items[0].mutate();
//CHECK:                     ^ mutating method cannot be called on immutable value
//CHECK:                     ^ mutating method 'mutate' cannot be called on an immutable value
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
    // A dynamic index into a swizzle cannot participate in whole-value writeback.
    incrementProjectedValue(storage.vectorValue.xy[index]);
//CHECK:                                          ^ argument must be l-value
//CHECK:                                          ^ argument passed to parameter '0' must be l-value.

    // A single tuple element likewise has no getter/setter writeback representation.
    incrementProjectedValue(storage.tupleValue._0);
//CHECK:                                       ^^ argument must be l-value
//CHECK:                                       ^^ argument passed to parameter '0' must be l-value.
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

[__NonCopyableType]
struct NonCopyableNestedStorageContainer
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

void warnForNestedRefOnInput(NestedStorageContainer input)
{
    incrementProjectedValue(input.value.value);
//CHECK:                                ^^^^^ mutating storage accessor called on `in` parameter
//CHECK:                                ^^^^^ mutating 'ref' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

void rejectNestedRefOnNonCopyableInput(NonCopyableNestedStorageContainer input)
{
    incrementProjectedValue(input.value.value);
//CHECK:                                ^^^^^ error E30445
//CHECK:                                ^^^^^ mutating 'ref' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}
