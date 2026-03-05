//TEST:SIMPLE: -target llvm-host-ir -emit-cpu-via-llvm

// This just needs to compile for the test to pass, because we can't actually
// run this with slang-rhi, as it won't link random external functions like
// this.
__extern_cpp int foo(Ptr<int> a, Ptr<int> b);

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    foo(nullptr, nullptr);
    return 0;
}
