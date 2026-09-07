// Coverage for the captured-`this` closure-field rename (issue #12923): a captured
// `this` is given a synthesized `$this` field name on every `this`-capture path, not
// only the interface default-method path. This exercises the plain-struct method path
// (whose field name changed from the receiver type's name to `$this`), a default method
// capturing `this` alongside a local (both the local-keeps-name and this->`$this` naming
// branches), and nested lambdas that each reference `this`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct S
{
    int x;
    int doubled() { let g = () => this.x * 2; return g(); }
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
    Impl impl = {};

    outputBuffer[0] = s.doubled();
    // CHECK: 42
    outputBuffer[1] = withLocal(impl);
    // CHECK: 121
    outputBuffer[2] = nested(impl);
    // CHECK: 22
}
