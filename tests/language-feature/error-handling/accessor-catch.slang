//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Accessors cannot be declared `throws`, but an error that is caught inside an accessor body
// does not escape it, so property getters and setters and `__subscript` getters may use
// `try` inside `do`/`catch`. Companion to accessor-uncaught-throw.slang (#13489).

// CHECK: 5
// CHECK-NEXT: -1
// CHECK-NEXT: 7
// CHECK-NEXT: 100
// CHECK-NEXT: 3
// CHECK-NEXT: -2

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Failed
}

int nonNegative(int n) throws MyError
{
    if (n < 0)
        throw MyError.Failed;
    return n;
}

struct S
{
    int x;

    property int checked
    {
        get
        {
            do
            {
                return try nonNegative(x);
            }
            catch (e : MyError)
            {
                return -1;
            }
        }
        set
        {
            do
            {
                x = try nonNegative(newValue);
            }
            catch (e : MyError)
            {
                x = 100;
            }
        }
    }

    __subscript(int i) -> int
    {
        get
        {
            do
            {
                return try nonNegative(x + i);
            }
            catch (e : MyError)
            {
                return -2;
            }
        }
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.x = 5;
    outputBuffer[0] = s.checked;
    s.x = -3;
    outputBuffer[1] = s.checked;
    s.checked = 7;
    outputBuffer[2] = s.x;
    s.checked = -1;
    outputBuffer[3] = s.x;
    s.x = 1;
    outputBuffer[4] = s[2];
    outputBuffer[5] = s[-5];
}
