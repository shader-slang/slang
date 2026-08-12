// This test is LLVM only due to it being the only backend where:
// * recursion is allowed
// * this bug manifests
//TEST:SIMPLE: -target llvm-host-ir -emit-cpu-via-llvm
struct Foo
{
    int f;
}

// This used to cause a compiler hang due to the lowerBufferElementTypes pass
// deferring the pointer load infinitely deep in the callstack.
void recursive(Foo* i)
{
    recursive(i);
}

export __extern_cpp int main(int argc, NativeString* argv)
{
    Foo* i = nullptr;
    recursive(i);
    return 0;
}
