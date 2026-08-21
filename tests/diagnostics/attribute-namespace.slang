// attribute-namespace.slang

// A user-defined attribute declared inside a `namespace` must be applicable through its qualified
// path `[my_namespace::Example(...)]`, including across a nested namespace. See
// shader-slang/slang#12668.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -no-codegen

namespace my_namespace
{
    [__AttributeUsage(_AttributeTargets.Function)]
    struct ExampleAttribute
    {
        int iParam;
    };
}

// A failed resolution would report the folded name 'my_namespace_Example'; asserting its absence
// confirms the qualified path resolved.
// CHECK-NOT: my_namespace_Example
[my_namespace::Example(1)]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

// A nested namespace exercises walking an intermediate qualifier segment.
namespace outer
{
    namespace inner
    {
        [__AttributeUsage(_AttributeTargets.Function)]
        struct NestedAttribute
        {
            int iParam;
        };
    }
}

// CHECK-NOT: outer_inner_Nested
[outer::inner::Nested(3)]
void usesNested()
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
