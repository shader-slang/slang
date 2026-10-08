//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A value read of abstract storage requires either a getter or a `ref` accessor. Semantic
// checking must reject setter-only storage before lowering tries to materialize the value.
struct SetterOnlyStorage
{
    property int propertyValue
    {
        [mutating]
        set {}
    }

    __subscript(int index) -> int
    {
        [mutating]
        set {}
    }
}

int readSetterOnlyProperty(SetterOnlyStorage storage)
{
    return storage.propertyValue;
//CHECK:           ^^^^^^^^^^^^^ property or subscript cannot be read
//CHECK:           ^^^^^^^^^^^^^ cannot read this property because it has no `get` or `ref` accessor
}

int readSetterOnlySubscript(SetterOnlyStorage storage)
{
    return storage[0];
//CHECK:          ^ property or subscript cannot be read
//CHECK:          ^ cannot read this subscript because it has no `get` or `ref` accessor
}

struct MutatingReadableStorage
{
    int backing;

    property int getterValue
    {
        [mutating]
        get { return backing; }
    }

    property int referenceValue
    {
        [mutating]
        ref { return backing; }
    }
}

Ref<int> acceptReferenceThroughRefAccessor(inout MutatingReadableStorage storage)
{
    return storage.referenceValue;
}

void checkDiscardedStorageReads()
{
    let storage = MutatingReadableStorage();

    // Discarding the result still evaluates the selected accessor. Both statements therefore
    // require a mutable receiver even though neither resulting `int` value has a consumer.
    storage.getterValue;
//CHECK:    ^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:    ^^^^^^^^^^^ mutating 'get' accessor cannot be called on an immutable value
    storage.referenceValue;
//CHECK:    ^^^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:    ^^^^^^^^^^^^^^ mutating 'ref' accessor cannot be called on an immutable value
}

void checkStorageReadsInControlFlowStatements()
{
    let storage = MutatingReadableStorage();

    // A switch selector evaluates the selected property value before choosing a case.
    switch (storage.getterValue)
//CHECK:            ^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:            ^^^^^^^^^^^ mutating 'get' accessor cannot be called on an immutable value
    {
    default:
        break;
    }

    // The increment expression is evaluated for its side effects after every loop iteration.
    for (; false; storage.getterValue)
//CHECK:                  ^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                  ^^^^^^^^^^^ mutating 'get' accessor cannot be called on an immutable value
    {}
}

[anyValueSize(16)]
interface IStoredValue
{}

struct StoredValue : IStoredValue
{}

IStoredValue globalStoredValue;

struct ExistentialStorage
{
    property IStoredValue value
    {
        [mutating]
        get { return globalStoredValue; }
    }
}

bool checkRuntimeExistentialTypeTest()
{
    let storage = ExistentialStorage();

    // A run-time `is` test evaluates the existential value before inspecting its dynamic type.
    // Receiver validation must therefore include the getter selected to obtain that value.
    return storage.value is StoredValue;
//CHECK:           ^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:           ^^^^^ mutating 'get' accessor cannot be called on an immutable value
}

struct GetterSetterStorage
{
    int backing;

    property int value
    {
        get { return backing; }
        set { backing = newValue; }
    }
}

Ref<int> rejectReferenceWithoutRefAccessor(GetterSetterStorage storage)
{
    // Forming a writable `Ref<int>` requires a direct address. A getter/setter pair supports
    // read-modify-write, but only a `ref` accessor can supply that address.
    return storage.value;
//CHECK:           ^^^^^ type mismatch in expression
//CHECK:           ^^^^^ expected an expression of type 'Ref<int, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>', got 'int'
}
