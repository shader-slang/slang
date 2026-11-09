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

[noinline]
Counter passThrough(Counter value)
{
    return value;
}

[noinline]
void update(Counter value, uint newValue)
{
    value.set(newValue);
}

[host]
void main()
{
    uint initialValue = hgbc_asm { result : $$uint = OpIntLit 3; };
    uint firstUpdate = hgbc_asm { result : $$uint = OpIntLit 7; };
    uint secondUpdate = hgbc_asm { result : $$uint = OpIntLit 11; };

    Counter original = new Counter();
    original.set(initialValue);
    Counter alias = passThrough(original);

    update(alias, firstUpdate);
    print(original.get());

    alias.set(secondUpdate);
    print(original.get());
}

// CHECK: 7
// CHECK-NEXT: 11
