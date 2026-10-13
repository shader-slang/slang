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

void discardSetterOnlyAssignment(inout SetterOnlyStorage storage)
{
    // The assignment has already completed the setter call before its l-value result is discarded.
    // Discarding that result must not introduce a read from the setter-only property.
    storage.propertyValue = 1;
}

void discardSetterOnlyAssignmentInForLoop(inout SetterOnlyStorage storage)
{
    // A `for` loop's side-effect expression has the same result-discarding semantics as an
    // expression statement.
    for (int i = 0; i < 1; storage.propertyValue = 2)
    {
        i++;
    }
}

int readSetterOnlyAssignmentResult(inout SetterOnlyStorage storage)
{
    // Consuming the assignment's l-value result requires reading the destination after the write.
    return (storage.propertyValue = 3);
//CHECK:            ^^^^^^^^^^^^^ property or subscript cannot be read
//CHECK:            ^^^^^^^^^^^^^ cannot read this property because it has no `get` or `ref` accessor
}

Ref<int> referenceSetterOnlyAssignmentResult(inout SetterOnlyStorage storage)
{
    // Forming a writable reference to the assignment result requires the destination's address.
    return (storage.propertyValue = 4);
//CHECK:            ^^^^^^^^^^^^^ property or subscript does not provide an address
//CHECK:            ^^^^^^^^^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

struct ReadableValue
{
    int value;

    [nonmutating]
    int read()
    {
        return value;
    }
}

struct SetterOnlyReceiverStorage
{
    property ReadableValue receiver
    {
        [mutating]
        set {}
    }
}

int invokeMethodOnSetterOnlyAssignmentResult(
    inout SetterOnlyReceiverStorage storage,
    ReadableValue replacement)
{
    // Immediate invocation consumes the selected method's receiver. The assignment result must
    // therefore provide the receiver value, which a setter-only property cannot do.
    return (storage.receiver = replacement).read();
//CHECK:            ^^^^^^^^ property or subscript cannot be read
//CHECK:            ^^^^^^^^ cannot read this property because it has no `get` or `ref` accessor
}

void retainMethodFromSetterOnlyAssignmentResult(
    inout SetterOnlyReceiverStorage storage,
    ReadableValue replacement)
{
    // Retaining the callable evaluates the assignment for its side effect but does not invoke the
    // selected method, so no receiver value is consumed here.
    let callable = (storage.receiver = replacement).read;
}

// Direct assignment requires only a write, while passing the subscript to `inout` requires both a
// read and a write. Both operations must diagnose the missing `set` or `ref` accessor.
struct GetterOnlyStorage
{
    property int propertyValue
    {
        get { return 0; }
    }

    __subscript(int index) -> int
    {
        get { return index; }
    }
}

void writeGetterOnlyProperty(GetterOnlyStorage storage)
{
    storage.propertyValue = 1;
//CHECK:    ^^^^^^^^^^^^^ property or subscript cannot be written
//CHECK:    ^^^^^^^^^^^^^ cannot write this property because it has no `set` or `ref` accessor
}

void incrementValue(inout int value) { value++; }

void modifyGetterOnlySubscript(GetterOnlyStorage storage)
{
    incrementValue(storage[0]);
//CHECK:                  ^ property or subscript cannot be written
//CHECK:                  ^ cannot write this subscript because it has no `set` or `ref` accessor
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

    property bool predicateValue
    {
        [mutating]
        get { return backing != 0; }
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
//CHECK:    ^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:    ^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value
    storage.referenceValue;
//CHECK:    ^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:    ^^^^^^^^^^^^^^ mutating method 'ref' cannot be called on an immutable value
}

void checkStorageReadInForLoopSideEffect()
{
    let storage = MutatingReadableStorage();

    // The increment clause is checked by a semantic-expression visitor dedicated to the loop side
    // effect. That visitor must still validate the receiver of the selected property accessor.
    for (; false; storage.getterValue)
//CHECK:                  ^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                  ^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value
    {}
}

void checkStorageReadInPredicate()
{
    let storage = MutatingReadableStorage();

    if (storage.predicateValue)
//CHECK:        ^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:        ^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value
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
//CHECK:           ^^^^^ mutating method cannot be called on immutable value
//CHECK:           ^^^^^ mutating method 'get' cannot be called on an immutable value
}

Optional<StoredValue> checkRuntimeExistentialCast()
{
    let storage = ExistentialStorage();

    // A run-time `as` cast obtains the existential value before inspecting its dynamic type.
    return storage.value as StoredValue;
//CHECK:           ^^^^^ mutating method cannot be called on immutable value
//CHECK:           ^^^^^ mutating method 'get' cannot be called on an immutable value
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
//CHECK:           ^^^^^ property or subscript does not provide an address
//CHECK:           ^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}

struct GetterSetterVectorStorage
{
    int4 backing;

    property int4 value
    {
        get { return backing; }
        set { backing = newValue; }
    }
}

void rejectDynamicIndexIntoAssignmentResult(
    inout GetterSetterVectorStorage storage,
    int4 replacement,
    int index)
{
    // Field and statically unique swizzle projections can read and write back through this
    // getter/setter pair. A dynamic index instead requires a direct address, so retaining the
    // assignment's destination does not make this property addressable.
    (storage.value = replacement)[index] = 1;
//CHECK:     ^^^^^ property or subscript does not provide an address
//CHECK:     ^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
}
