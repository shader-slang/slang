//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/refined_miss_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -r tests/ray-tracing-2/frontend/module/refined-miss-invoke-cache.slang-module -no-codegen

import slang.raytracing;
import refined_miss_library;

// The serialized file has a different name from the imported module so the first run always
// imports source. The second run explicitly loads that file and must recover invoke's stage
// identity through the inherited IMissShader conformance, not only the direct IRefinedMiss base.
extension RefinedMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}

// An extension conformance owns the implementation outside the stage struct declaration.
extension ExtensionMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}

// The receiver and the selected implementation both carry this concrete payload substitution.
extension GenericMiss<uint>
{
    void callDirectly(in rt::MissInput<GenericMissContext<uint>> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}

// The implementation is supplied by a generic extension of a refined stage interface. Its
// stage role must survive serialization without having to check the provider's body again.
extension DefaultMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}

// Both receivers select the same generic method declaration. Only one receiver conforms to a
// stage interface, so classifying the bare method declaration would incorrectly reject this call.
extension OrdinaryInvokeProvider
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
    }
}

// The method's generic extension only constrains IInvokeProvider. Recover its stage role from
// this receiver's selected conformance rather than from the implementation's lexical parent.
extension SharedDefaultMiss
{
    void callDirectly(in rt::MissInput<RefinedMissContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}
