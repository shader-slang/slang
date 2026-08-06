// precompiled-spirv-pointer-param.slang

// A test that uses slang-modules with embedded precompiled SPIRV and a library containing 
// a function with a pointer parameter.
// The test compiles a library slang (module-library-pointer-param.slang) with -embed-downstream-ir then links the
// library to entrypoint slang (this file).
// The test passes if there is no errror thrown.
// TODO: Check if final linkage used only the precompiled spirv.

//TEST:COMPILE: tests/library/module-library-pointer-param.slang -o tests/library/module-library-pointer-param.slang-module -target spirv -embed-downstream-ir -incomplete-library
// The link step below deliberately runs with SPIR-V validation enabled: it is the
// regression assertion for shader-slang/slang#12371. Do not add -skip-spirv-validation
// to quiet a failure here without establishing that the emitted SPIR-V is actually valid.
//TEST:COMPILE: tests/library/precompiled-spirv-pointer-param.slang -target spirv -stage anyhit -entry anyhit -o tests/library/linked.spirv

import "module-library-pointer-param";

struct Payload
{
    int val;
}

struct Attributes
{
    float2 bary;
}

[vk::push_constant] int* g_int;

[shader("anyhit")]
void anyhit(inout Payload payload, Attributes attrib)
{
    payload.val = ptrFunc(g_int);
}
