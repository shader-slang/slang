//TEST:SIMPLE: -experimental-feature -entry Miss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/stage_input_copy_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/stage-input-copy-cache.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/stage-input-copy-cache.slang-module -entry Miss -no-codegen

import slang.raytracing.portable;
import stage_input_copy_library;

// The helper copies the stage input in its body. Both source and serialized imports must retain
// a normal helper call accepting the view supplied explicitly by the selected stage.
struct Miss : rt::IMissShader
{
    typealias Context = input_copy::Context;

    static void invoke(in rt::MissInput<Context> input)
    {
        let local = input;
        input_copy::updatePayload(rt::MissInput<Context>(local));
    }
}
