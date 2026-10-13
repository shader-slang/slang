//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Semantic checking selects a property or subscript accessor according to how the reference's
// value is consumed. Each use below puts an immutable receiver behind a different access path, so
// the diagnostic names the exact accessor that lowering would call.

static int externalValue;
static int2 externalVector;

void takeIn(int value) {}
void takeInOut(inout int value) { value++; }
void takeConstRef(__constref int value) {}
void takeRef(__ref int value) { value++; }

struct PropertyStorage
{
    int value;

    property int readPrefersGetter
    {
        [mutating] get { return value; }
        [nonmutating] ref { return externalValue; }
    }

    property int writePrefersSetter
    {
        [mutating] set {}
        [nonmutating] ref { return externalValue; }
    }

    property int referenceRequiresRef
    {
        [nonmutating] set {}
        [mutating] ref { return value; }
    }

    property int readWriteValue
    {
        [mutating] get { return value; }
        [mutating] set { value = newValue; }
    }

    property int2 componentUsesRef
    {
        [nonmutating] get { return 0; }
        [nonmutating] set {}
        [mutating] ref { return externalVector; }
    }

    property int2 componentUsesReadWrite
    {
        [mutating] get { return 0; }
        [nonmutating] set {}
    }
}

void checkPropertyAccessors()
{
    let storage = PropertyStorage();

    // Semantic accessor checking selects the getter for a read even when a `ref` accessor could
    // also provide the value.
    takeIn(storage.readPrefersGetter);
//CHECK:           ^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:           ^^^^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value

    // Semantic accessor checking selects the setter for a whole-value write even when a `ref`
    // accessor could accept the write.
    storage.writePrefersSetter = 1;
//CHECK:    ^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:    ^^^^^^^^^^^^^^^^^^ mutating method 'set' cannot be called on an immutable value

    // For a direct reference, semantic accessor checking requires the `ref` accessor and never
    // substitutes the setter.
    takeRef(storage.referenceRequiresRef);
//CHECK:            ^^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:            ^^^^^^^^^^^^^^^^^^^^ mutating method 'ref' cannot be called on an immutable value

    // For a projected write, semantic accessor checking selects the `ref` accessor when one exists
    // because lowering needs the address of the complete property value before selecting its
    // component.
    storage.componentUsesRef.x = 1;
//CHECK:    ^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:    ^^^^^^^^^^^^^^^^ mutating method 'ref' cannot be called on an immutable value

    // Without a `ref` accessor, semantic accessor checking selects the getter and setter for a
    // projected write. Lowering reads the complete value, changes the component, and writes the
    // complete value back.
    storage.componentUsesReadWrite.x = 1;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value

    // For a projected borrow, semantic accessor checking likewise selects a `ref` accessor when
    // available and otherwise reads the projected value into a temporary.
    takeConstRef(storage.componentUsesRef.x);
//CHECK:                 ^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                 ^^^^^^^^^^^^^^^^ mutating method 'ref' cannot be called on an immutable value
    takeConstRef(storage.componentUsesReadWrite.x);
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value

    // A temporary receiver cannot satisfy the mutating getter selected for a read.
    takeIn(PropertyStorage().readPrefersGetter);
//CHECK:                     ^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                     ^^^^^^^^^^^^^^^^^ mutating method 'get' cannot be called on an immutable value
}

void checkPropertyReadWrite(PropertyStorage storage)
{
    // A read/write argument reads through the getter and writes back through the setter. The
    // function parameter provides l-value storage, so both mutating accessors are valid. Receiver
    // validation emits the established warning for each mutation of the local copy of an `in`
    // parameter.
    takeInOut(storage.readWriteValue);
//CHECK:              ^^^^^^^^^^^^^^ mutating method called on `in` parameter
//CHECK:              ^^^^^^^^^^^^^^ mutating method 'get' called on `in` parameter 'storage'
//CHECK:              ^^^^^^^^^^^^^^ mutating method called on `in` parameter
//CHECK:              ^^^^^^^^^^^^^^ mutating method 'set' called on `in` parameter 'storage'
}

struct SubscriptStorage
{
    int value;

    __subscript(uint index) -> int
    {
        [mutating] get { return value; }
        [nonmutating] ref { return externalValue; }
    }

    __subscript(int index) -> int
    {
        [mutating] set {}
        [nonmutating] ref { return externalValue; }
    }

    __subscript(int2 index) -> int
    {
        [nonmutating] set {}
        [mutating] ref { return value; }
    }

    __subscript(uint2 index) -> int
    {
        [mutating] get { return value; }
        [mutating] set { value = newValue; }
    }
}

void checkSubscriptAccessors()
{
    let storage = SubscriptStorage();

    // Overload resolution selects a different subscript declaration from each index type. Semantic
    // accessor checking then selects `get` for a read, `set` for an assignment, and `ref` for a
    // writable reference.
    takeIn(storage[uint(0)]);
//CHECK:          ^ mutating method cannot be called on immutable value
//CHECK:          ^ mutating method 'get' cannot be called on an immutable value
    storage[int(0)] = 1;
//CHECK:   ^ mutating method cannot be called on immutable value
//CHECK:   ^ mutating method 'set' cannot be called on an immutable value
    takeRef(storage[int2(0)]);
//CHECK:           ^ mutating method cannot be called on immutable value
//CHECK:           ^ mutating method 'ref' cannot be called on an immutable value
}

void checkSubscriptReadWrite(SubscriptStorage storage)
{
    // As with a property, a subscript passed to `inout` reads through its getter and writes back
    // through its setter. Receiver validation warns for both mutating accessors because `storage`
    // is a local copy.
    takeInOut(storage[uint2(0)]);
//CHECK:             ^ mutating method called on `in` parameter
//CHECK:             ^ mutating method 'get' called on `in` parameter 'storage'
//CHECK:             ^ mutating method called on `in` parameter
//CHECK:             ^ mutating method 'set' called on `in` parameter 'storage'
}
