//TEST:SIMPLE: -target llvm-host-ir -emit-cpu-via-llvm

struct Foo
{
    int a;
    float3 b;
};

// This just needs to compile for the test to pass, because we can't actually
// run this with slang-rhi, as it won't link random external functions like
// this.
__extern_cpp int foo(Ptr<Foo> a, Ptr<Foo> b);

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    foo(nullptr, nullptr);
    return 0;
}
