//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Each throwing call inside a `try` operand carries its own `try` (#13489), and the error from
// either call reaches the enclosing `catch`.

// CHECK: 6
// CHECK-NEXT: -1
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 8
// CHECK-NEXT: -1
// CHECK-NEXT: 5
// CHECK-NEXT: 7

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Failed
}

int f(int n) throws MyError
{
    if (n < 0)
        throw MyError.Failed;
    return n;
}

int g(int x) throws MyError { return x * 2; }

struct S
{
    int x;
    __subscript(int i) -> int { get { return x + i; } }
}

struct T
{
    int v;
    static T make(int n) throws MyError
    {
        T t;
        t.v = try f(n);
        return t;
    }
    int get() throws MyError { return try f(v); }
}

int nested(int n) throws MyError { return try g(try f(n)); }
int constructorConversion(int n) throws MyError { return int(try f(n)); }
int castConversion(int n) throws MyError { return (int)(try f(n)); }
int subscriptOperand(int n) throws MyError
{
    S s;
    s.x = 1;
    return try g(s[try f(n)]);
}
int memberCalls(int n) throws MyError
{
    T t = try T.make(n);
    return try t.get();
}

int run(int which, int n)
{
    do
    {
        switch (which)
        {
        case 0: return try nested(n);
        case 1: return try constructorConversion(n);
        case 2: return try castConversion(n);
        case 3: return try subscriptOperand(n);
        default: return try memberCalls(n);
        }
    }
    catch (e : MyError)
    {
        return -1;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = run(0, 3);
    outputBuffer[1] = run(0, -1);
    outputBuffer[2] = run(1, 3);
    outputBuffer[3] = run(2, 4);
    outputBuffer[4] = run(3, 3);
    outputBuffer[5] = run(3, -1);
    outputBuffer[6] = run(4, 5);
    outputBuffer[7] = run(4, 7);
}
