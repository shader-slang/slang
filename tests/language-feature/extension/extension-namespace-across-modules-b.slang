//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain

// Test case for issue #9157: extension not finding identifier from same namespace across module boundary
import "extension-namespace-across-modules-a.slang";

namespace ns
{
    struct B
    {
    }

    // This should work - A should be found from the same namespace in the imported module
    // Before the fix, this would give error 30015: undefined identifier 'A'
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