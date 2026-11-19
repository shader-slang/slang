// Accessing a field inherited from a struct base through a property or a
// subscript with backing storage used to abort with E99997 "unexpected member
// flavor": the base upcast defers a keyed-field projection into a
// BoundMemberInfo, and the consumers of that info only accepted VarDecl
// members, rejecting the InheritanceDecl of a struct base.
//
// The Counter/ICounter conformance case guards a different invariant:
// interface bases must keep lowering to witness-table requirements rather
// than fields, so interface conformance stays untouched by the field-like
// path. The Counting scenarios at the end pin the accessor call counts of a
// deferred inherited-field write (two gets: the temporary materialization
// and the write-back merge) against the one-get own-field analog.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

#pragma warning(disable:30816)

struct Base
{
    float value;

    [mutating]
    void bump(float delta)
    {
        value += delta;
    }
}

struct Payload : Base
{
    float extra;
}

// Two levels of struct inheritance: `value` is two bases up from Leaf.
struct Middle : Base
{
    float m;
}

struct Leaf : Middle
{
    float l;
}

struct Holder
{
    Payload stored = {};
    Payload stored2 = {};
    Leaf leafStored = {};
    Payload array[2] = {};

    property Payload payload
    {
        get { return stored; }
        set { stored = newValue; }
    }

    property Payload payload2
    {
        get { return stored2; }
        set { stored2 = newValue; }
    }

    property Leaf leafProp
    {
        get { return leafStored; }
        set { leafStored = newValue; }
    }

    __subscript(int index) -> Payload
    {
        get { return array[index]; }
        set { array[index] = newValue; }
    }
}

void addFive(inout Base b)
{
    b.value += 5.0;
}

interface ICounter
{
    property count : int { get; set; }
}

struct Counter : ICounter
{
    int c;

    property count : int
    {
        get { return c; }
        set(newValue) { c = newValue; }
    }
}

int readCount(ICounter counter)
{
    return counter.count;
}

static int gGets = 0;
static int gSets = 0;

struct Counting
{
    Payload stored = {};

    property Payload counted
    {
        get { gGets += 1; return stored; }
        set { gSets += 1; stored = newValue; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=8):name=buf
RWStructuredBuffer<Payload> buf;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Holder h = {};

    // Write and read an inherited field through a get/set property.
    h.payload.value = 3.0;
    outputBuffer[0] = h.payload.value; // CHECK: 3.0

    // Read-modify-write through a get/set property, observed on the backing field.
    h.payload.value += 10.0;
    outputBuffer[1] = h.stored.value; // CHECK: 13.0

    // An inherited [mutating] method called through a get/set property.
    h.payload.bump(2.0);
    outputBuffer[2] = h.payload.value; // CHECK: 15.0

    // An inherited field two bases up, through a get/set property.
    h.leafProp.value = 4.0;
    outputBuffer[3] = h.leafStored.value; // CHECK: 4.0

    // Write and read an inherited field through a subscript with accessors.
    h[1].value = 5.0;
    outputBuffer[4] = h[1].value; // CHECK: 5.0

    // Write and read an inherited field on a buffer element.
    buf[0].value = 7.0;
    outputBuffer[5] = buf[0].value; // CHECK: 7.0

    // Upcast of the property value to the base type.
    h.payload2.value = 6.0;
    Base b = h.payload2;
    outputBuffer[6] = b.value; // CHECK: 6.0

    // Passing the property value to an `inout` parameter of the base type.
    h.payload2.value = 1.0;
    addFive(h.payload2);
    outputBuffer[7] = h.stored2.value; // CHECK: 6.0

    // A struct conforming to an interface keeps working: interface bases lower
    // to witness-table requirements, never fields, so the field-like path
    // must leave them untouched.
    Counter counter = {9};
    outputBuffer[8] = readCount(counter); // CHECK: 9

    // An inherited-field write through a get/set property reads the property
    // twice (the temporary materialization and the write-back merge) and
    // writes it once; the own-field write below reads and writes it once.
    Counting counting = {};
    counting.counted.value = 8.0;
    outputBuffer[9] = float(gGets); // CHECK: 2
    outputBuffer[10] = float(gSets); // CHECK: 1
    outputBuffer[11] = counting.counted.value; // CHECK: 8.0

    counting.counted.extra = 9.0;
    outputBuffer[12] = float(gGets); // CHECK: 4
    outputBuffer[13] = float(gSets); // CHECK: 2

    // The deferred write-back merges the modified base subobject with the
    // sibling fields of the property value: `extra` survives the write.
    Holder s = {};
    s.payload.extra = 5.0;
    s.payload.value = 8.0;
    outputBuffer[14] = s.payload.extra; // CHECK: 5.0
    outputBuffer[15] = s.payload.value; // CHECK: 8.0
}
