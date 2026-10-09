//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Throwing calls inside `try` operands run and propagate their errors (#13489). A throwing call in
// the operand of another call carries its own `try`. A `try` on an identity conversion, as in
// `try int(f(n))`, covers the throwing call it converts. Each `run` returns -1 when the error
// reaches its `catch`.

// CHECK: 6
// CHECK-NEXT: -1
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 8
// CHECK-NEXT: -1
// CHECK-NEXT: 5
// CHECK-NEXT: 7
// CHECK-NEXT: 9
// CHECK-NEXT: -1
// CHECK-NEXT: 2
// CHECK-NEXT: -1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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
int constructorIdentity(int n) throws MyError { return try int(f(n)); }
int castIdentity(int n) throws MyError { return try (int)f(n); }

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
        case 4: return try memberCalls(n);
        case 5: return try constructorIdentity(n);
        default: return try castIdentity(n);
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
    outputBuffer[0] = run(0, 3);   // g(f(3)) = 6
    outputBuffer[1] = run(0, -1);  // f throws
    outputBuffer[2] = run(1, 3);   // int(f(3)) = 3
    outputBuffer[3] = run(2, 4);   // (int)f(4) = 4
    outputBuffer[4] = run(3, 3);   // g(s[f(3)]) = g(4) = 8
    outputBuffer[5] = run(3, -1);  // f throws
    outputBuffer[6] = run(4, 5);   // T.make(5).get() = 5
    outputBuffer[7] = run(4, 7);   // T.make(7).get() = 7
    outputBuffer[8] = run(5, 9);   // try int(f(9)) = 9
    outputBuffer[9] = run(5, -1);  // f throws through the identity conversion
    outputBuffer[10] = run(6, 2);  // try (int)f(2) = 2
    outputBuffer[11] = run(6, -1); // f throws through the identity cast
}
