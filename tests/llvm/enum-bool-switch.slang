//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program

// Regression test for shader-slang/slang#12298: a `switch` on an `enum : bool` must
// compile on the LLVM (CPU) target, whose `switch` accepts the i1 selector directly
// (no integer legalization). The runtime selector keeps the switch from folding away
// so it reaches the LLVM emitter, where it lowers to a `switch i1`.

enum Mode : bool
{
    Immediate = false,
    Deferred = true,
}

int classify(Mode m)
{
    switch (m)
    {
    case Mode::Immediate:
        return 10;
    case Mode::Deferred:
        return 20;
    }
    return -1;
}

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    Mode m = Mode(argc > 1);
    // CHECK: switch i1
    printf("%d\n", classify(m));
    return 0;
}
