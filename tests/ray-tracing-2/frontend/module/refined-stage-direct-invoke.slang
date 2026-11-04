//TEST:SIMPLE: -experimental-feature -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/refined_miss_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module -no-codegen

import slang.raytracing.portable;
import refined_miss_library;

// The serialized file has a different name from the imported module so the first run always
// imports source. The second run explicitly loads that file. Both must allow forwarding an
// existing input to the inherited IMissShader implementation.
extension RefinedMiss
{
    static void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        RefinedMiss::invoke(input);
    }
}

// An extension conformance owns the implementation outside the stage struct declaration.
extension ExtensionMiss
{
    static void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        ExtensionMiss::invoke(input);
    }
}

// The stage type and the selected implementation both carry this concrete payload substitution.
extension GenericMiss<uint>
{
    static void callDirectly(in rt::MissInput<GenericMissContext<uint>> input)
    {
        GenericMiss<uint>::invoke(input);
    }
}

// A generic extension of a refined stage interface supplies the implementation. Ordinary
// method lookup must still find it after the provider has been serialized.
extension DefaultMiss
{
    static void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        DefaultMiss::invoke(input);
    }
}

// Both types select the same generic static method declaration. The call is allowed whether
// or not the type also conforms to a stage interface.
extension OrdinaryInvokeProvider
{
    static void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        OrdinaryInvokeProvider::invoke(input);
    }
}

// The method's generic extension only constrains IInvokeProvider; this type also uses it
// to implement the stage interface.
extension SharedDefaultMiss
{
    static void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        SharedDefaultMiss::invoke(input);
    }
}
