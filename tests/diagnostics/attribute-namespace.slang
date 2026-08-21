// attribute-namespace.slang

// Regression test for shader-slang/slang#12668: a user-defined attribute declared inside a
// `namespace` can be applied through its qualified path `[my_namespace::Example(...)]`, not only
// via `using namespace` + the unqualified form. Before the fix the parser folded the qualified
// name into the flat identifier `my_namespace_Example`, the attribute failed to resolve, and the
// compiler emitted `warning E31000: unknown attribute` and silently dropped the attribute.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -no-codegen

namespace my_namespace
{
    [__AttributeUsage(_AttributeTargets.Function)]
    struct ExampleAttribute
    {
        int iParam;
    };
}

// The qualified path must resolve the in-namespace attribute: the folded name for a failed
// resolution ('my_namespace_Example') must never appear in the diagnostics.
// CHECK-NOT: my_namespace_Example
[my_namespace::Example(1)]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

// Negative control: a genuinely undefined qualified attribute must still warn, proving the new
// scoped fallback does not silence real errors.
// CHECK: warning{{.*}}31000
// CHECK: unknown attribute 'my_namespace_Nonexistent'
[my_namespace::Nonexistent(1)]
void other()
{
}
