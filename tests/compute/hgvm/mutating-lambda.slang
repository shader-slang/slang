//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

class Counter
{
    uint value;

    __init() {}

    [mutating]
    void set(uint newValue)
    {
        value = newValue;
    }

    uint get()
    {
        return value;
    }
}

[host]
uint invoke<TFunc : IMutatingFunc<uint>>(inout TFunc function)
{
    return function();
}

[host]
void main()
{
    uint capturedState = 6;
    var statefulCallback = ()! =>
    {
        capturedState++;
        return capturedState;
    };

    print(invoke(statefulCallback));
    print(invoke(statefulCallback));
    print(capturedState);

    let counter = new Counter();
    let classCallback = () => counter.set(13);
    classCallback();
    print(counter.get());
}

// CHECK: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 6
// CHECK-NEXT: 13
