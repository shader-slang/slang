// Additional coverage for capturing `this` in a lambda (issue #12923): the captured
// receiver is reachable through the synthesized closure on every `this`-capture path.
// A plain struct method captures `this`; an extension method captures `this`; an
// interface default method captures `this` together with a local variable in one lambda;
// and a `this` captured only by an inner nested lambda still propagates out through the
// enclosing lambda.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct S
{
    int x;
    int doubled() { let g = () => this.x * 2; return g(); }
}

struct E
{
    int y;
}
extension E
{
    int tripled() { let g = () => this.y * 3; return g(); }
}

interface IFoo
{
    int getVal();

    int computeWithLocal()
    {
        int bonus = 100;
        let f = () => this.getVal() + bonus;
        return f();
    }

    int computeNested()
    {
        let outer = () =>
        {
            let inner = () => this.getVal();
            return inner() + 1;
        };
        return outer();
    }
}

struct Impl : IFoo { int getVal() { return 21; } }

int withLocal<T : IFoo>(T v) { return v.computeWithLocal(); }
int nested<T : IFoo>(T v) { return v.computeNested(); }

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.x = 21;
    E e;
    e.y = 5;
    Impl impl = {};

    outputBuffer[0] = s.doubled();
    // CHECK: 42
    outputBuffer[1] = e.tripled();
    // CHECK: 15
    outputBuffer[2] = withLocal(impl);
    // CHECK: 121
    outputBuffer[3] = nested(impl);
    // CHECK: 22
}
