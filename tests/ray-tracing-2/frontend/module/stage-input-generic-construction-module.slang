// Checks that a generic helper in an imported module is diagnosed once a caller binds a stage
// input to its parameter, both when the helper comes from source and from a serialized module.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -ignore-capabilities -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/stage_input_generic_construction_library.slang -experimental-feature -ignore-capabilities -o tests/ray-tracing-2/frontend/module/stage-input-generic-construction-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=SERIALIZED,non-exhaustive): -experimental-feature -ignore-capabilities -r tests/ray-tracing-2/frontend/module/stage-input-generic-construction-cache.slang-module -no-codegen

import slang.raytracing.portable;
import stage_input_generic_construction_library;

// The helper module only declares the generics; binding a stage input here is what makes the
// forbidden construction concrete, in the main module as well as through a serialized import.
struct Miss : rt::IMissShader
{
    typealias Context = generic_construction::Context;

    static void invoke(in rt::MissInput<Context> input)
    {
        input.payload.value += generic_construction::makeFreshInput<rt::MissInput<Context>>();
        //CHECK: fresh construction of a structural ray-tracing stage input
        input.payload.value += generic_construction::hopOne<rt::MissInput<Context>>();
        //CHECK: fresh construction of a structural ray-tracing stage input
        //SERIALIZED: fresh construction of a structural ray-tracing stage input
        //SERIALIZED: fresh construction of a structural ray-tracing stage input
    }
}
