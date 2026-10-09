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
