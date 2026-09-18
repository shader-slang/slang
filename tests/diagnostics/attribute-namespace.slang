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

// A leading `::` roots the qualified attribute name at the module (global) scope; the folded
// spelling of `::my_namespace::Example` would be '_my_namespace_Example'.
// CHECK-NOT: _my_namespace_Example
[::my_namespace::Example(4)]
void usesGlobalQualified()
{
}

// The exact struct name resolves on the first, unsuffixed attempt (the name as written, before the
// `Foo`->`FooAttribute` retry) — a spelling the legacy flat lookup never resolved. Its folded form
// would be 'my_namespace_ExampleAttribute'.
// CHECK-NOT: my_namespace_ExampleAttribute
[my_namespace::ExampleAttribute(5)]
void usesExactStructName()
{
}

// Negative control: a genuinely undefined qualified attribute must still warn — the qualified-name
// fallback must not silence real errors.
// CHECK: warning{{.*}}31000
// CHECK: unknown attribute 'my_namespace_Nonexistent'
[my_namespace::Nonexistent(1)]
void other()
{
}
