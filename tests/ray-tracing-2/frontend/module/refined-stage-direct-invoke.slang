//TEST:SIMPLE: -experimental-feature -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/refined_miss_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module -no-codegen

import slang.raytracing;
import refined_miss_library;

// The serialized file has a different name from the imported module so the first run always
// imports source. The second run explicitly loads that file. Both must allow forwarding an
// existing input to the inherited IMissShader implementation.
extension RefinedMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}

// An extension conformance owns the implementation outside the stage struct declaration.
extension ExtensionMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}

// The receiver and the selected implementation both carry this concrete payload substitution.
extension GenericMiss<uint>
{
    void callDirectly(in rt::MissInput<GenericMissContext<uint>> input)
    {
        this.invoke(input);
    }
}

// A generic extension of a refined stage interface supplies the implementation. Ordinary
// method lookup must still find it after the provider has been serialized.
extension DefaultMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}

// Both receivers select the same generic method declaration. The call is allowed whether or
// not the receiver also conforms to a stage interface.
extension OrdinaryInvokeProvider
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}

// The method's generic extension only constrains IInvokeProvider; this receiver also uses it
// to implement the stage interface.
extension SharedDefaultMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}
