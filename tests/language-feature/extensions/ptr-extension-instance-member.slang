// Instance members that an extension adds to a pointer type can be called on a pointer
// value with `.`. Member lookup tries the pointee type first, so a pointee member wins
// over an extension member with the same name. Inside an extension member, `this` is
// the pointer and `This` is the pointer type.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: set intPtr = ubuffer(data=[10 20 30 40], stride=4);
uniform int* intPtr;

struct S
{
    int value;
}

//TEST_INPUT: set sPtr = ubuffer(data=[5], stride=4);
uniform S* sPtr;

struct Shadow
{
    int value;
    int get() { return value + 1; }
}

//TEST_INPUT: set shadowPtr = ubuffer(data=[7], stride=4);
uniform Shadow* shadowPtr;

//TEST_INPUT: set ptrSlot = ubuffer(data=[0 0], stride=4);
uniform Ptr<int>* ptrSlot;

//TEST_INPUT: set tuplePtr = ubuffer(data=[3 4], stride=4);
uniform Tuple<int, int>* tuplePtr;

interface IThing
{
    int f(This other);
}

extension Ptr<int>
{
    int loadPlusOne() { return *this + 1; }

    [mutating]
    void advance() { this = this + 1; }
}

extension<T> Ptr<T>
{
    T loadGeneric() { return *this; }
}

extension Ptr<S>
{
    int valueTimesTwo() { return (*this).value * 2; }
}

extension int*
{
    int second() { return *(this + 1); }
}

extension Ptr<int> : IThing
{
    int f(This other) { return *other - *this; }
}

extension Ptr<Shadow>
{
    int get() { return -1; }
}

// When several pointer levels have the member, the innermost one wins, as in `lookUpMember`.
extension Ptr<int>
{
    int level() { return 1; }
    property int plusTen { get { return *this + 10; } }
    property int value
    {
        get { return *this; }
        [nonmutating] set { *this = newValue; }
    }
    static int answer() { return 77; }
}

extension Ptr<Ptr<int>>
{
    int level() { return 2; }
    int outerOnly() { return **this + 100; }
}

int pointeeOfGeneric<T>(Ptr<T> pt)
{
    return int(sizeof(pt.loadGeneric()));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Ptr<int> p = intPtr;
    Ptr<int> q = intPtr + 3;
    Ptr<S> ps = sPtr;
    Ptr<Shadow> pShadow = shadowPtr;

    // CHECK: 11
    outputBuffer[0] = p.loadPlusOne();

    // CHECK-NEXT: 10
    outputBuffer[1] = p.loadGeneric();

    // CHECK-NEXT: 10
    outputBuffer[2] = ps.valueTimesTwo();

    // CHECK-NEXT: 20
    outputBuffer[3] = p.second();

    // CHECK-NEXT: 30
    outputBuffer[4] = p.f(q);

    // CHECK-NEXT: 8
    outputBuffer[5] = pShadow.get();

    Ptr<int> cursor = p;
    cursor.advance();
    // CHECK-NEXT: 20
    outputBuffer[6] = *cursor;

    // A member found on an inner pointer level is called with that level, `*pp`, as `this`.
    *ptrSlot = intPtr + 2;
    Ptr<Ptr<int>> pp = ptrSlot;
    // CHECK-NEXT: 31
    outputBuffer[7] = pp.loadPlusOne();
    // CHECK-NEXT: 31
    outputBuffer[8] = (*pp).loadPlusOne();
    // CHECK-NEXT: 31
    outputBuffer[9] = pp->loadPlusOne();

    Ptr<Tuple<int, int>> pTuple = tuplePtr;
    // CHECK-NEXT: 4
    outputBuffer[10] = pTuple.loadGeneric()._1;

    // CHECK-NEXT: 1
    outputBuffer[11] = pp.level();
    // CHECK-NEXT: 130
    outputBuffer[12] = pp.outerOnly();
    // CHECK-NEXT: 30
    outputBuffer[13] = pp.loadGeneric();
    // CHECK-NEXT: 20
    outputBuffer[14] = p.plusTen;
    // CHECK-NEXT: 4
    outputBuffer[15] = pointeeOfGeneric(p);

    // A mutating member found on the inner level writes the pointer stored at `*pp`.
    pp.advance();
    // CHECK-NEXT: 40
    outputBuffer[16] = **ptrSlot;

    // A static member is found through the instance, and a setter on the inner level writes
    // through `*pp`, which now points at `intPtr[3]`.
    // CHECK-NEXT: 77
    outputBuffer[17] = p.answer();
    pp.value = 33;
    // CHECK-NEXT: 33
    outputBuffer[18] = intPtr[3];
}
