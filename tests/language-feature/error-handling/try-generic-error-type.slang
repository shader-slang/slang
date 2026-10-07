//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A `try` on a callee whose error type is generic checks the caller and any `catch` against
// the error type substituted at the call site: `g<MyError>` propagates and is caught as
// `MyError`, and so does `viaAssoc<Thrower>`, whose `throws U.E` becomes `MyError`.

enum MyError
{
    Failed = 7
}

interface IThrower
{
    associatedtype E;
    int get(int x) throws E;
}

struct Thrower : IThrower
{
    typealias E = MyError;
    int get(int x) throws MyError
    {
        if (x < 0)
            throw MyError.Failed;
        return x * 2;
    }
}

int g<E>(E e, int x) throws E
{
    if (x < 0)
        throw e;
    return x + 1;
}

int viaGeneric(int x) throws MyError { return try g<MyError>(MyError.Failed, x); }

int viaAssoc<U : IThrower>(U u, int x) throws U.E { return try u.get(x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    do { outputBuffer[0] = try viaGeneric(1); } catch (e: MyError) { outputBuffer[0] = -1; }
    do { outputBuffer[1] = try viaGeneric(-1); } catch (e: MyError) { outputBuffer[1] = int(e); }
    do { outputBuffer[2] = try g<MyError>(MyError.Failed, -5); } catch (e: MyError) { outputBuffer[2] = 100 + int(e); }
    Thrower t;
    do { outputBuffer[3] = try viaAssoc(t, -3); } catch (e: MyError) { outputBuffer[3] = 200 + int(e); }
}

// CHECK: type: int32_t
// CHECK-NEXT: 2
// CHECK-NEXT: 7
// CHECK-NEXT: 107
// CHECK-NEXT: 207
