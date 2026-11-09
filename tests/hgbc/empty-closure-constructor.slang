//TEST:INTERPRET(hgvm):
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

// Host calls preserve the closure's type and constructor. Inlining an ordinary
// call can erase the empty closure and hide an uninitialized constructor result.
[host]
int apply<T : IFunc<int, int>>(T transform, int value)
{
    return transform(value);
}

[host]
int applyGeneric<T>(int value)
{
    return apply((int x) => x + 1, value);
}

void check(int actual, int expected)
{
    if (actual != expected)
    {
        let message = "Incorrect closure result";
        hgbc_asm { OpFail $message; };
    }
}

[host]
void main()
{
    check(apply((int x) => x + 1, 41), 42);
    check(applyGeneric<float>(41), 42);

    // Nonempty closures must still initialize and retain their captured fields.
    int offset = 7;
    check(apply((int x) => x + offset, 35), 42);
}

// CHECK: OpMakeStruct
