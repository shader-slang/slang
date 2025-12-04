//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain

// Test for extension keyword finding identifier from same namespace across module boundary
// This reproduces issue #9157

// Module a.slang content (simulated via namespace)
namespace ns
{
    public interface A
    {
        void func();
    }
}

// Module b.slang content - extension should find interface A from same namespace
namespace ns
{
    struct B
    {
    }

    // This should work - A should be found from the same namespace
    extension B: A
    {
        void func() {}
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ns::B b;
    // CHECK: void main()
}