import "tests/language-feature/extension/extension-namespace-across-modules-a.slang";

namespace ns
{
    struct B {}

    // Test the workaround mentioned in the issue
    extension B: ns.A
    {
        void func() {}
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ns::B b;
}